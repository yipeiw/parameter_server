#pragma once
#include "linear_method/batch_scheduler.h"
#include "linear_method/countlrl1_common.h"
namespace PS {
namespace LM {

class CountLrL1Scheduler : public BatchScheduler, public CountLrL1Common {
 public:
  virtual void init();

  virtual void runIteration();
  virtual void showProgress(int iter);
 protected:
  void showKKTFilter(int iter);
  void showRoundFilter(int iter);
  void showLR(int iter);

  //double kkt_filter_threshold_;
};

} // namespace LM
} // namespace PS
