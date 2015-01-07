#pragma once
#include "linear_method/batch_server.h"
#include "linear_method/smooth_common.h"
namespace PS {
namespace LM {

class SmoothServer : public BatchServer, public SmoothCommon {
 public:
  virtual void preprocessData(const MessagePtr& msg);
  virtual void iterate(const MessagePtr& msg) { updateWeight(msg); }
  virtual void evaluateProgress(Progress* prog);
 protected:
  void updateWeight(const MessagePtr& msg);
  //void updateWeight(int grp, SizeR range, SArray<double> Delta);
  void updateWeight(int grp, SizeR range, SArray<double> G, SArray<double> U);

  double kkt_filter_threshold_;
  double violation_;
};

} // namespace LM
} // namespace PS
