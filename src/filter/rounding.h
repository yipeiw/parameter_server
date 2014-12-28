#pragma once
#include "filter/filter.h"
#include <math.h>
#include <random>

namespace PS {

class RoundingFilter : public Filter {
  public:
   RoundingFilter() {
     bit_num_ = max_bit_;
     std::vector<int> bit_num_ = {1,2,4,8,16,32};
     for(auto& bitNum : bit_num_){
       bit_choice_[bitNum] = std::pow(2, -bitNum);
     }
   }
  
   void set_bit(int bit) {
     bit_num_ = bit;
   }

   int get_bit() {
     return bit_num_;
   }
   
   void adaptBit(int iter){
	float threshold = 1.0/(iter*iter);
	for(auto& choice : bit_choice_){
	  if(choice.second <= threshold){
	    bit_num_ = choice.first;
	    return; 
          }
	}	
	bit_num_ = max_bit_;
   }

   template <typename T> T randomizedRound(T& v) {
    auto epsilon = std::pow(2, -bit_num_);
    auto upper = (std::ceil(v/epsilon))*epsilon;
    auto lower = (std::floor(v/epsilon))*epsilon;
    auto threshold = v-lower;

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    auto randNum = distribution(generator);

    if (randNum < threshold) {
      return upper;
    } else {
      return lower;
    }
   }
  
  private:
    int bit_num_;
    std::map<int, double> bit_choice_;
    const int max_bit_ = 32;
};

} // namespace PS
