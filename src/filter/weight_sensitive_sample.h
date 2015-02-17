#pragma once
#include "util/shared_array_inl.h"
#include <random>
#include <math.h>

namespace PS{

template <typename V> 
int partition(SArray<V>& arr, size_t left, size_t right, int pivotPos);
template <typename V>
V findKthLargest(SArray<V>& arr, size_t left, size_t right, int K);
template <typename V>
V ApproximateKth(SArray<V>& arr, int K);

std::random_device rd;

template <typename V> 
void WeightSensitiveSample(SArray<V>& value, int K) {
    if (value.size()==K) return;
    if (value.size()==0) {
      for(int i=0; i<value.size(); i++) {value[i]=0; }
    }

    SArray<V> sort_arr(value.size(), 0.0);
    for(int i=0; i< value.size(); i++) {
	sort_arr[i] = fabs(value[i]);
    }
    auto threshold = findKthLargest(sort_arr, 0, value.size()-1, K);
    //LL << "the K th largest value " << threshold << "\n";
    //LL << "sorted_arr "<<sort_arr;
    
    //auto threshold = ApproximateKth(value, K);

    //std::uniform_real_distribution<float> distribution(0.0, 1.0); 
    for(int i = 0; i < value.size(); i++) {
      if (value[i]==0) continue;
      //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      //std::default_random_engine generator(seed);
	
      if (fabs(value[i]) < threshold) value[i] = 0;
    }
}

template <typename V>
V ApproximateKth(SArray<V>& value, int K) {
  V max_val = value[0], min_val = value[0];
  for (int i=1; i < value.size(); ++i) {
    V cur_val = value[i];
    if (cur_val > max_val) max_val = cur_val;
    if (cur_val < min_val) min_val = cur_val;
  }
  //LL << "max "<<max_val  << " min " <<min_val;

  int histNum = 20;
  int hist[20];
  float ratio = 1.0/histNum;
  //LL << "ratio " <<ratio;
  
  //memset(hist, 0, histNum*sizeof(int));
  for (int i=0; i < histNum; i++) hist[i] = 0;

  //LL << "setup histogram";
  int ave = (int) std::floor(value.size()*ratio);
  //LL << "ave " << ave;

  V r = 1/(max_val-min_val+1e-10);
  LL << "range "<< r;
  for (int i=0; i < value.size(); ++i) {
    hist[(int) std::floor((value[i]-min_val)*r*histNum)] += 1;
  }

  for (int i = 0; i<histNum; ++i) {
    if(hist[i]>0) LL << i << " histNum:" << hist[i];
  }

  int acc = 0;
  for (int i = histNum-1; i>=0; i--) {
    acc += hist[i];
    if (acc > K-ave) { 
      return static_cast<V>((i+0.5)*ratio/r+min_val);
    }
  }
  return max_val;
}

template <typename V> 
V findKthLargest(SArray<V>& value, size_t left, size_t right, int K) {
    //sorted in descending order
    //LL << "search from " << left << " to "<<right << ", "<<value.size();
    //std::random_device rd;
    std::uniform_int_distribution<int> distribution(left, right);
    size_t pivotPos = distribution(rd);
    //LL << "pivot " << pivotPos;
    //LL << " val " << value[pivotPos];

    while (1) {
      size_t pos = partition(value, left, right, pivotPos);
      //LL << "rank:" << pos;

      int diff = pos+1-K;
      if (diff==0) {
        return value[pos];
      } else if (diff > 0) {
	return findKthLargest(value, left, pos-1, K);
      } else {
        return findKthLargest(value, pos+1, right, K);
      }
    }
}

template <typename V> 
size_t partition(SArray<V>& arr, size_t left, size_t right, size_t pivotPos) {
    if (left==right) {
        return left;
    }

    auto pivot = arr[pivotPos];
    std::swap(arr[pivotPos], arr[right]);
    size_t tag = left;

    for (size_t i=left; i<right+1; i++) {
      if (arr[i] > pivot) {
	  std::swap(arr[tag], arr[i]);
          tag ++;
      }
    }
    std::swap(arr[tag], arr[right]);
    return tag;
}


}
