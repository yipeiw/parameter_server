#pragma once
#include "linear_method/batch_worker.h"
#include "linear_method/countlrl1_common.h"
namespace PS {
namespace LM {

class CountLrL1Worker : public BatchWorker, public CountLrL1Common {
 public:
  virtual void preprocessData(const MessagePtr& msg);
  virtual void iterate(const MessagePtr& msg) { computeGradient(msg); }
  virtual void evaluateProgress(Progress* prog);
 protected:
  void computeGradient(const MessagePtr& msg);
  void computeAndPushGradient(int time, Range<Key> g_key_range, int grp, SizeR col_range);
  void computeGradient(int grp, SizeR col_range, SArray<double> G);
  void pullAndUpdateDual(
      int time, Range<Key> g_key_range, int grp, SizeR col_range, const MessagePtr& msg);
  void updateDual(int grp, SizeR col_range, SArray<double> new_weight);
  void updateDual(int grp, SizeR row_range, SizeR col_range, SArray<double> w_delta);
};

} // namespace LM
} // namespace PS
