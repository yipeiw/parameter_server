#pragma once
#include "linear_method/batch_scheduler.h"
#include "linear_method/lrl1_common.h"
namespace PS {
namespace LM {

class LrL1Scheduler : public BatchScheduler, public LrL1Common {
 public:
  virtual void init();

  virtual void runIteration();
  virtual void showProgress(int iter);
 protected:
  void showKKTFilter(int iter);

  //double kkt_filter_threshold_;
};

} // namespace LM
} // namespace PS
