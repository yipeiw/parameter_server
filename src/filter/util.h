#pragma once
#include "filter/filter.h"
#include "string.h"
#include <bitset>

namespace PS {

bool marked(float v) {return v!=v;}
bool marked(double v) {return v!=v;}

template <typename V>
int Sparse(SArray<V>& A) {
  int total = 0;
  for(int i=0; i< A.size(); i++) {
    std::bitset<sizeof(V)*8> x(A[i]);
    total += x.count();
  }  
  return total;
}

template <typename V>
int NNZVector(SArray<V>& A) {
  size_t nnz = A.size();
  for(int i =0; i < A.size(); i++) {
    if (A[i]==0) nnz--;
  }
  return nnz;
}

template <typename V, typename T> 
bool FindExtreme(SArray<V>& arr, T* maxV, T* minV) {
  bool start = false;
  for(int i=0; i<arr.size(); i++) {
    V val = arr[i];
    if (marked(val) || val==0) continue;
    if(!start) {
      *maxV = val;
      *minV = val;
      start=true;
      continue;
    }

    if(val > *maxV) {*maxV = val;continue;}
    if(val < *minV) {*minV = val;}
  }     
  return start;
}

}
