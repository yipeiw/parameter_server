#pragma once
#include "filter/sparse_filter.h"
namespace PS{

class SampleFilter : public SparseFilter{
 public:
  template <typename V> sample(SArray<V>& value) {
  
  }
 private:
  int expected_num_;
};

}
