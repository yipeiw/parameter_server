#pragma once
#include "linear_method/batch_server.h"
#include "linear_method/lrl1_common.h"
namespace PS {
namespace LM {

class LrL1Server : public BatchServer, public LrL1Common {
 public:
  virtual void preprocessData(const MessagePtr& msg);
  virtual void iterate(const MessagePtr& msg) { updateWeight(msg); }
  virtual void evaluateProgress(Progress* prog);
 protected:
  void updateWeight(const MessagePtr& msg);
  void updateWeight(int grp, SizeR range, SArray<double> G, SArray<double> U);

  double max_lr_ = 0.0;
  double min_lr_ = 1.0;

  std::unordered_map<int, SArray<int>> count_;
  //double kkt_filter_threshold_;
  //double violation_;
};

} // namespace LM
} // namespace PS
