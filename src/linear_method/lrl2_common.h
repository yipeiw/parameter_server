#pragma once
#include "base/bitmap.h"
#include "filter/kkt_filter.h"
#include "filter/rounding.h"
#include "filter/sample_filter.h"

namespace PS {
namespace LM {

class LrL2Common {
 public:
  double newDelta(double delta_max, double delta_w) {
    return std::min(delta_max, 2 * fabs(delta_w) + .1);
  }
 protected:
  //std::unordered_map<int, Bitmap> active_set_;
  std::unordered_map<int, SArray<double>> NW_;

  std::unordered_map<int, SArray<double>> delta_;
  //KKTFilter kkt_filter_;
  RoundingFilter randomround_filter_;
  SampleFilter sample_filter_;
  //bool using_kkt_filter_ = false;
  bool using_round_filter_ = false;
  bool using_sample_filter_ = false;
};

} // namespace LM
} // namespace PS
