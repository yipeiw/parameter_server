#pragma once
#include "filter/sparse_filter.h"
#include "filter/sample.h"

#include "math.h"

namespace PS{

class KKTFilter : public SparseFilter{
  public:
    updateThreshold();
  private:
    double kkt_filter_threshold_ = 1e20;
};

}
