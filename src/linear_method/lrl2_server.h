#pragma once
#include "linear_method/batch_server.h"
#include "linear_method/lrl2_common.h"
namespace PS {
namespace LM {

class LrL2Server : public BatchServer, public LrL2Common {
 public:
  virtual void preprocessData(const MessagePtr& msg);
  virtual void iterate(const MessagePtr& msg) { updateWeight(msg); }
  virtual void evaluateProgress(Progress* prog);
 protected:
  void updateWeight(const MessagePtr& msg);
  void updateWeight(int grp, SizeR range, SArray<double> D);

  double max_lr_ = 0.0;
  double min_lr_ = 1.0;
  double max_delta_ = 0.0;
  int nzz = 0;
  //double kkt_filter_threshold_;
  //double violation_;
};

} // namespace LM
} // namespace PS
