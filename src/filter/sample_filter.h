#pragma once
#include "filter/weight_sensitive_sample.h"
#include "filter/filter.h"
#include "math.h"
#include <random>

namespace PS{

class SampleFilter{
 public:
  SampleFilter() {
    //LL << "generate randomlist";
    random_real_list = new double[RandomLen];
    std::random_device rd;
    std::uniform_real_distribution<float> distribution(0,1);
    for (int i=0; i<RandomLen; i++) {
      random_real_list[i] = 1.0/distribution(rd);
    }
    //LL << "finish generation";
  }
  ~SampleFilter() {
    delete[] random_real_list;
  }

  bool marked(float v) {return v!=v;}
  bool marked(double v) {return v!=v;}
 
  template<typename V>
  void PrioritySample(SArray<V>& value, float samplePercent) {
    //LL << "start prepare data";
    int count = 0;
    SArray<V> priority_arr(value.size(), 0.0);
    SArray<V> sort_arr(value.size(),0.0);
    for(int i=0; i < value.size(); i++) {
      V val = value[i];
      if (marked(val) || val==0) continue;
      priority_arr[i] = fabs(val)*random_real_list[used % RandomLen];
      sort_arr[i] = priority_arr[i];
      count++;
      used++;
    }
    //LL<<"data copied";
    used = used % RandomLen;
    
    if (count < 2) {LL<<"effective "<<count<<",no need to sample"; return;}

    int K = count*samplePercent;
    LL << "priority sampleFilter "<< K << " from "<<count<<" of "<<value.size();
    
    if (K==0) {
      for(int i=0; i<value.size(); i++) {
        if (marked(value[i]) || value[i]==0) continue;
        value[i]=0;
      }
      return;
    }

    auto threshold = findKthLargest(sort_arr, 0, count-1, K+1);
    LL << "threshold:" << threshold;

    sort_arr.clear();

    for(int i=0; i < priority_arr.size(); i++) {
      if (priority_arr[i]==0) {continue;} //include value[i]=0 or nan
      if (priority_arr[i]<=threshold) {
        value[i]=0;
      } else {
        V val = value[i];
        int sgn=val>0?1:-1;
        value[i] = fabs(val)>threshold ? val:sgn*threshold;
      }
    }
    priority_arr.clear();
  }

 private:
  double* random_real_list = nullptr;
  int RandomLen = 10000;
  int used = 0;
};

}
