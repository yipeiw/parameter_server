#include "linear_method/lrl2_scheduler.h"
#include "base/matrix_io.h"
#include "base/sparse_matrix.h"
namespace PS {
namespace LM {

void LrL2Scheduler::init() {
  BatchScheduler::init();

  if (conf_.lrl2().has_round_filter() ) {
    using_round_filter_ = true;
    auto round_conf = conf_.lrl2().round_filter();
    randomround_filter_.init(round_conf);
  }
  LL << "use round filter " << using_round_filter_;
  /* 
  if (conf_.lrl2().has_kkt_filter()) {
    using_kkt_filter_ = true;
    kkt_filter_.set_ratio(conf_.lrl2().kkt_filter().threshold_ratio());
  }
  LL << "use kkt_filter " << using_kkt_filter_;*/

  if (conf_.lrl2().has_sample_filter()) {
    using_sample_filter_ = true;
    auto sample_conf = conf_.lrl2().sample_filter();
    sample_filter_.init(sample_conf);
  }
  LL << "use sample filter " << using_sample_filter_;
}


void LrL2Scheduler::runIteration() {
  CHECK(conf_.has_lrl2());
  CHECK_EQ(conf_.loss().type(), LossConfig::LOGIT);
  CHECK_EQ(conf_.penalty().type(), PenaltyConfig::L2);
  auto sol_cf = conf_.solver();
  int tau = sol_cf.max_block_delay();
 
  //kkt_filter_threshold_ = 1e20; 
  //float samplePercent = 0.1; //for sample_filter baseline test
  //bool reset_kkt_filter = false;
 
  bool random_blk_order = sol_cf.random_feature_block_order();
  if (!random_blk_order) {
    LI << "Warning: Randomized block order often acclerates the convergence.";
  }
  LI << "Train l_1 logistic regression by " << tau << "-delayed block coordinate descent";

  // iterating
  int max_iter = sol_cf.max_pass_of_data();
  auto pool = taskpool(kActiveGroup);
  // pick up a large enough time stamp to avoid any possible conflict
  int time = std::max(10000, pool->time() + (int)fea_grp_.size() * 10);
  const int first_time = time + 1;
 
  for (int iter = 0; iter < max_iter; ++iter) {
    // pick up a updating order
    // TODO avoid some tau ...
   
    //randomround_filter_.adaptBit(iter);
    //fprintf(stderr, "rounding bit num:%d\n", randomround_filter_.get_bit());

    auto order = blk_order_;
    if (random_blk_order) std::random_shuffle(order.begin(), order.end());
    if (iter == 0) order.insert(
            order.begin(), prior_blk_order_.begin(), prior_blk_order_.end());

    // iterating on feature blocks
    for (int i = 0; i < order.size(); ++i) {
      Task update = newTask(Call::UPDATE_MODEL);
      auto cmd = set(&update);
      //round filter
      if (using_round_filter_ && i==0) {
        cmd->set_roundfilter_bit_num(randomround_filter_.get_bit());
      }

      if (using_sample_filter_ && i ==0 ) {
        cmd->set_sample_percent(sample_filter_.getPercent());
      }

      // kkt filter
     /* if (using_kkt_filter_ && i==0) {
          cmd->set_kkt_filter_threshold(kkt_filter_.get_threshold());
          if (reset_kkt_filter) cmd->set_reset_kkt_filter(true);
      }*/
      // block info
      auto blk = fea_blk_[order[i]];
      blk.second.to(cmd->mutable_key());
      cmd->add_fea_grp(blk.first);

      // time stamp
      update.set_time(time+1);
      int wait_time = time - tau;
      // force zero delay for important feature blocks
      if (iter == 0 && i < prior_blk_order_.size()) {
        wait_time = time;
      }
      // make sure leading UPDATE_MODEL tasks could be picked up by workers
      if (wait_time < first_time) {
        wait_time = Message::kInvalidTime;
      }
      update.add_wait_time(wait_time);

      time = pool->submit(update);
    }

    LL << "evaluate " << iter;
    // evaluate the progress
    Task eval = newTask(Call::EVALUATE_PROGRESS);
    if (time - tau >= first_time) {
      eval.add_wait_time(time - tau);
    }
    time = pool->submitAndWait(
        eval, [this, iter](){ Scheduler::mergeProgress(iter); });
    showProgress(iter);

    if (using_round_filter_) {
        randomround_filter_.updateParam(iter);
    }

    /*if (using_sample_filter_) {
	sample_filter_.updateParam(iter, g_progress_[iter].delta_minimax());
    }*/

   /*
   if (using_kkt_filter_) {
     double vio = g_progress_[iter].violation();
     kkt_filter_.update_threshold(vio, g_train_info_.num_ex());
     //LL <<"r:"<<kkt_filter_.get_ratio()<<" vio:"<<vio<<" threshold:"<<kkt_filter_.get_threshold();  
   }*/
    // update the kkt filter strategy
    /*double vio = g_progress_[iter].violation();
    double ratio = conf_.darling().kkt_filter_threshold_ratio();
    kkt_filter_threshold_ = vio / (double)g_train_info_.num_ex() * ratio;
    */

    // check if finished
    double rel = g_progress_[iter].relative_objv();
    if (rel > 0 && rel <= sol_cf.epsilon()) {
      /*
      if (using_kkt_filter_) {
        if (reset_kkt_filter) {
          LI << "Stopped: relative objective <= " << sol_cf.epsilon();
          break;
        } else {
          reset_kkt_filter = true;
        } 
      } else {*/
        LI << "Stopped: relative objective <= " << sol_cf.epsilon();
        break;
      /*}
   } else {
     if(using_kkt_filter_) { reset_kkt_filter = false; }*/
   }

    if (iter == max_iter - 1) {
      LI << "Reached maximal " << max_iter << " data passes";
    }
  }
}

/*void LrL2Scheduler::showKKTFilter(int iter) {
  if (iter == -3) {
    fprintf(stderr, "|      KKT filter     ");
  } else if (iter == -2) {
    fprintf(stderr, "| threshold  #activet ");
  } else if (iter == -1) {
    fprintf(stderr, "+---------------------");
  } else {
    auto prog = g_progress_[iter];
    fprintf(stderr, "| %.1e %11llu ", kkt_filter_.get_threshold(), (uint64)prog.nnz_active_set());
  }
}*/

void LrL2Scheduler::showLR(int iter) {
  if (iter == -3) {
    fprintf(stderr, "| Learning Rate ");
  } else if (iter == -2) {
    fprintf(stderr, "| max   min  ");
  } else if (iter == -1) {
    fprintf(stderr, "+-----------");
  } else {
    auto prog = g_progress_[iter];
    fprintf(stderr, "| %.2e %.5e ", prog.max_lr(), prog.min_lr());
  }
}

void LrL2Scheduler::showSampleFilter(int iter) {
  if (iter == -3) {
    fprintf(stderr, "|      Sample filter     ");
  } else if (iter == -2) {
    fprintf(stderr, "| threshold ");
  } else if (iter == -1) {
    fprintf(stderr, "+------");
  } else {
    fprintf(stderr, "| %.3e ", sample_filter_.getThreshold());
  }
}

void LrL2Scheduler::showRoundFilter(int iter) {
  if (iter == -3) {
    fprintf(stderr, "|      Round filter     ");
  } else if (iter == -2) {
    fprintf(stderr, "| bitNum ");
  } else if (iter == -1) {
    fprintf(stderr, "+------");
  } else {
    fprintf(stderr, "| %d ", randomround_filter_.get_bit());
  }
}

void LrL2Scheduler::showProgress(int iter) {
  int s = iter == 0 ? -3 : iter;
  for (int i = s; i <= iter; ++i) {
    showObjective(i);
    showNNZ(i);
    //showLR(i);
    //if (using_kkt_filter_) { showKKTFilter(i); }
    if (using_round_filter_) {showRoundFilter(i);}
    if (using_sample_filter_) {showSampleFilter(i);}
    showTime(i);
  }
}

} // namespace LM
} // namespace PS
