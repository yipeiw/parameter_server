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
void getAbsArray(SArray<V>& arr, SArray<V>& absArr);

template <typename V> void sampleThresholdCut(SArray<V>& value, int K) {
    if (K < 0 || K > value.size()) {
	LL << "error K " << K;
	return;
    }
    if (K==0) {
        value.setZero();
        return;
    }
    if (K==value.size()) {
        return;
    }

    SArray<V> sort_arr(value.size());
    getAbsArray(value, sort_arr);
    auto threshold = findKthLargest(sort_arr, 0, sort_arr.size()-1, K);
    LL << "the K th largest value " << threshold << "\n";
    LL << sort_arr;
 
    for(size_t i = 0; i < value.size(); i++) {
	if (fabs(value[i]) < threshold) {
		value[i] = 0.0;
	}
    }
}

template <typename V> 
V findKthLargest(SArray<V>& value, size_t left, size_t right, int K) {
    //sorted in descending order
    std::random_device rd;
    std::uniform_int_distribution<int> distribution(left, right);
    size_t pivotPos = distribution(rd);
    //LL << "pivot postion:" << pivotPos << ", pivot value:" << value[pivotPos];

    while (1) {
      size_t pos = partition(value, left, right, pivotPos);
      //LL << "element rank " << pos+1 << " in finding top " << K;
      //LL << "sorted array" << value;

      int diff = pos+1-K;
      //LL << "diff: " << diff;	
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

template <typename V>
void getAbsArray(SArray<V>& arr, SArray<V>& absArr) {
    for (size_t i = 0; i < arr.size(); i++) {
        if (arr[i] >= 0 ) {
           absArr[i] = arr[i];
        } else {
            absArr[i] = -1*arr[i];
        }
    }
}

}
