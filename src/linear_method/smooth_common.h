#pragma once
#include "base/bitmap.h"
#include "filter/sparse_filter.h"
#include "filter/rounding.h"
#include "filter/sample_filter.h"

namespace PS {
namespace LM {

class SmoothCommon {
 public:
  double newDelta(double delta_max, double delta_w) {
    return std::min(delta_max, fabs(delta_w)+.01);
  }
 protected:
  std::unordered_map<int, Bitmap> active_set_;
  std::unordered_map<int, SArray<double>> delta_;
  SparseFilter kkt_filter_;
  RoundingFilter randomround_filter_;
  SampleFilter sample_filter_;
};

} // namespace LM
} // namespace PS
