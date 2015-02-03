#pragma once
#include "filter/sparse_filter.h"
#include "filter/sample.h"
#include "linear_method/linear_method.pb.h"

#include <random>
#include "math.h"

namespace PS{

class SampleFilter : public SparseFilter{
 public:
  void init(LM::SampleFilterConfig& sample_conf) {
      setThreshold(sample_conf.threshold());
  }

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
  void setThreshold(double p) {threshold_ = p; }
  double getThreshold() {return threshold_;}

  template<typename T> void apply(T& v) {
      std::default_random_engine generator;
      std::uniform_real_distribution<float> distribution(0.0, 1.0);
      auto randNum = distribution(generator);

      double val = fabs(v);
      if (val/randNum > threshold_) {
        int sign = v>0? 1:-1;
        v = sign*std::max(val, threshold_);
      } else {
        v = 0;
      }
  }

  void updateParam(int iter, double refer_val) {
      double ratio = 0.8;
      if (iter==0) {
        threshold_ = ratio*refer_val;
      } else {
        threshold_ = std::min(ratio*threshold_/(iter*iter), refer_val);
      }
  }

 private:
  int expected_num_;
  bool setNumTag=false;
  float expect_percent_;
  bool setPercentTag = false;
  double threshold_ = 0.0;
};

}
