#pragma once
#include "filter/sparse_filter.h"
#include "filter/sample.h"

#include "math.h"

namespace PS{

class SampleFilter : public SparseFilter{
 public:
  template <typename V> 
  void sample(SArray<V>& value, int leftIdx, int rightIdx) {
      //threshold cut method, other sampling need to be added
      if (setPercentTag){
          expected_num_=round( (rightIdx-leftIdx)*expect_percent_);
          setNumTag = true;  
      } 
      if (setNumTag) {
          sampleThresholdCut(value, leftIdx, rightIdx, expected_num_);
      } else {
          LL << "sample percent not given";
      }  
}

  void setPercent(float p) {expect_percent_ = p; setPercentTag = true;}
  void setSampleNum(int num) { expected_num_ = num; setNumTag = true;}

 private:
  int expected_num_;
  bool setNumTag=false;
  float expect_percent_;
  bool setPercentTag = false;
};

}
