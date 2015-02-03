#include "linear_method/lrl1_server.h"
namespace PS {
namespace LM {

void LrL1Server::preprocessData(const MessagePtr& msg) {
  BatchServer::preprocessData(msg);
  for (int grp : fea_grp_) {
    size_t n = model_->key(grp).size();
    delta_[grp].resize(n, conf_.lrl1().delta_init_value());
    //active_set_[grp].resize(n, true);
  }

  if (conf_.lrl1().has_kkt_filter()) {
    using_kkt_filter_ = true;
    kkt_filter_.init(model_, fea_grp_);
  }
  //LL << "server kkt " << using_kkt_filter_;

  if (conf_.lrl1().has_round_filter() ) {
    using_round_filter_ = true;
    auto round_conf = conf_.lrl1().round_filter();
    randomround_filter_.init(round_conf);
  }
  //LL << "server round:" << using_round_filter_;
}

void LrL1Server::updateWeight(const MessagePtr& msg) {
  int time = msg->task.time() * k_time_ratio_;
  auto cmd = get(msg);
  
  //round filter update
  if (cmd.has_roundfilter_bit_num()) {
    randomround_filter_.set_bit(cmd.roundfilter_bit_num()); 
  }

  /*if (cmd.has_sample_filter_percent()) {
     sample_filter_.setPercent(cmd.sample_filter_percent()); 
  }*/
  if (cmd.has_kkt_filter_threshold()) {
    kkt_filter_.set_threshold(cmd.kkt_filter_threshold());
    kkt_filter_.violation_ = 0;
  }
  if (cmd.reset_kkt_filter()) {
    kkt_filter_.reset(fea_grp_);
    //for (int grp : fea_grp_) active_set_[grp].fill(true);
  }

  CHECK_EQ(cmd.fea_grp_size(), 1);
  int grp = cmd.fea_grp(0);
  Range<Key> g_key_range(cmd.key());
  auto col_range = model_->find(grp, g_key_range);

  // none of my bussiness
  if (model_->myKeyRange().setIntersection(g_key_range).empty()) return;

  //  aggregate all workers' local gradients
  model_->waitInMsg(kWorkerGroup, time);

  // update the weights
  if (!col_range.empty()) {
    auto data = model_->received(time);
    CHECK_EQ(col_range, data.first);
    CHECK_EQ(data.second.size(), 2);

    sys_.hb().startTimer(HeartbeatInfo::TimerType::BUSY);
    updateWeight(grp, col_range, data.second[0], data.second[1]);
    sys_.hb().stopTimer(HeartbeatInfo::TimerType::BUSY);
  }

  model_->finish(kWorkerGroup, time+1);
}

void LrL1Server::updateWeight(
    int grp, SizeR range, SArray<double> G, SArray<double> U) {
  CHECK_EQ(G.size(), range.size());
  CHECK_EQ(U.size(), range.size());

  double eta = conf_.learning_rate().eta();
  double lambda = conf_.penalty().lambda(0);
  double delta_max = conf_.lrl1().delta_max_value();
  auto& value = model_->value(grp);

  //auto& active_set = active_set_[grp];
  auto& delta = delta_[grp];

  for (size_t i = 0; i < range.size(); ++i) {
    size_t k = i + range.begin();
    //if (!active_set.test(k)) continue;
    if (using_kkt_filter_ && !kkt_filter_.test(grp, k)) continue;

    double g = G[i], u = U[i] / eta + 1e-10;
    double g_pos = g + lambda, g_neg = g - lambda;
    double& w = value[k];
    double d = - w, vio = 0;

    if (using_kkt_filter_) {
      if (w == 0) {
        double kkt_threshold = kkt_filter_.get_threshold();
        if (g_pos < 0) {
          vio = - g_pos;
        } else if (g_neg > 0) {
          vio = g_neg;
        } else if (g_pos > kkt_threshold && g_neg < - kkt_threshold) {
          kkt_filter_.clear(grp, k);
          kkt_filter_.mark(&w);
          continue;
        }
      }
      kkt_filter_.violation_ = std::max(kkt_filter_.violation_, vio);
    }

    if (g_pos <= u * w) {
      d = - g_pos / u;
    } else if (g_neg >= u * w) {
      d = - g_neg / u;
    }
    d = std::min(delta[k], std::max(-delta[k], d));
    delta[k] = newDelta(delta_max, d);
    w += d;

    double lr = fabs(1.0/u);
    max_lr_ = std::max(max_lr_, lr);
    min_lr_ = std::min(min_lr_, lr);

    if (w!= 0 && using_round_filter_) {
      w = randomround_filter_.randomizedRound(w);
    } 
  }

  //sample_filter_.sample(value, range.begin(), range.end());
}

void LrL1Server::evaluateProgress(Progress* prog) {
  size_t nnz_w = 0;
  size_t nnz_as = 0;
  double objv = 0;
  for (int grp : fea_grp_) {
    const auto& value = model_->value(grp);
    for (double w : value) {
      if (using_kkt_filter_ && kkt_filter_.marked(w)) continue;
      if (w == 0) continue;

      ++ nnz_w;
      objv += fabs(w);
    }
    if (using_kkt_filter_) nnz_as += kkt_filter_.get_nnz(grp);
  }
  prog->add_objv(objv * conf_.penalty().lambda(0));
  prog->set_nnz_w(nnz_w);

  prog->set_max_lr(max_lr_);
  prog->set_min_lr(min_lr_);


  if (using_kkt_filter_) {
    prog->set_violation(kkt_filter_.violation_);
    prog->set_nnz_active_set(nnz_as);
  }

}

} // namespace LM
} // namespace PS
