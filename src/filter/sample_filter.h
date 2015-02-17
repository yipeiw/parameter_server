#pragma once
#include "filter/sparse_filter.h"
#include "filter/weight_sensitive_sample.h"
#include "linear_method/linear_method.pb.h"

#include <random>
#include "math.h"

namespace PS{

class SampleFilter : public SparseFilter{
 public:
  void init(LM::SampleFilterConfig& sample_conf) {
      //setThreshold(sample_conf.threshold());
      setPercent(sample_conf.percent());
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
  double getPercent() { return expect_percent_; }

  void setSampleNum(int num) { expected_num_ = num; setNumTag = true;}
  void setThreshold(double p) {threshold_ = p; }
  double getThreshold() {return threshold_;}

  template<typename T> void apply(SArray<T> Val) {
      //Random(Val);
      int K = std::floor(expect_percent_*Val.size());
      //LL << "sample " << expect_percent_ << ",num "<<K << " of "<<Val.size();
      WeightSensitiveSample(Val, K);
      /*std::default_random_engine generator;
      std::uniform_real_distribution<float> distribution(0.0, 1.0);
      auto randNum = distribution(generator);

      double val = fabs(v);
      if (val/randNum > threshold_) {
        int sign = v>0? 1:-1;
        v = sign*std::max(val, threshold_);
      } else {
        v = 0;
      }*/
  }

  template<typename T> void Random(SArray<T> Val) {
   for (int i=0; i< Val.size(); i++) {
     unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
     std::default_random_engine generator(seed);
     std::uniform_real_distribution<float> distribution(0.0, 1.0);
     auto randNum = distribution(generator);
     if (randNum >= expect_percent_) Val[i]=0;
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
