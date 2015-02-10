#pragma once
#include "base/shared_array_inl.h"
#include <random>
#include <math.h>

namespace PS{

template <typename V> 
int partition(SArray<V>& arr, size_t left, size_t right, int pivotPos);
template <typename V>
V findKthLargest(SArray<V>& arr, size_t left, size_t right, int K);

template <typename V> 
void WeightSensitiveSample(SArray<V> value, int K) {
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

    std::uniform_real_distribution<float> distribution(0.0, 1.0); 
    for(int i = 0; i < value.size(); i++) {
      if (value[i]==0) continue;
      //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      //std::default_random_engine generator(seed);
	
      if (fabs(value[i]) < threshold) value[i] = 0;
      /*
      if (fabs(value[i]/distribution(generator)) < threshold) {
        value[i] = 0.0;
      } else {
        int sign = value[i] > 0 ? 1:-1;
        V Val = fabs(value[i]);
        value[i] = sign*std::max(Val, threshold);
      }*/
    }
}

template <typename V> 
V findKthLargest(SArray<V>& value, size_t left, size_t right, int K) {
    //sorted in descending order
    std::random_device rd;
    std::uniform_int_distribution<int> distribution(left, right);
    size_t pivotPos = distribution(rd);

    while (1) {
      size_t pos = partition(value, left, right, pivotPos);

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
