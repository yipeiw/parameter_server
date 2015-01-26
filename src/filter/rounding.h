#pragma once
#include "filter/filter.h"
#include "linear_method/linear_method.pb.h"
#include <math.h>
#include <random>

namespace PS {

enum Strategy {FIXED, ADAPT};
enum RoundMethod {RANDOM, HARD};

class RoundingFilter : public Filter {
  public:
   RoundingFilter() {}

   void init(LM::RoundFilterConfig& conf_) {
     bit_num_ = max_bit_;
     std::vector<int> bit_num_ = {1,5,13,32};
     for(auto& bitNum : bit_num_){
       bit_choice_[bitNum] = std::pow(2, -bitNum);
     }

     if (conf_.type() == LM::RoundFilterConfig::FIXED) {
       filter_type_ = Strategy::FIXED;
       set_bit(conf_.bit_num());
     } else {
       filter_type_ = Strategy::ADAPT;
     }
     if (conf_.rtype()==LM::RoundFilterConfig::RANDOM) {
       round_type_ = RoundMethod::RANDOM;
     } else {
       round_type_ = RoundMethod::HARD;
     }

   }
 
   void updateParam(int iter) {
     if (filter_type_==Strategy::FIXED) {
       return;
     } else {
       adaptBit(iter);
     }
   }
 
   void set_bit(int bit) {
     bit_num_ = bit;
   }

   int get_bit() {
     return bit_num_;
   }
  
   //resolution_t \leq r_t*eta, and \sum_t r_t = o(T) 
   void adaptBit(int iter, double ratio=1.0){
	float threshold = ratio * 1.0/(iter*iter);
	for(auto& choice : bit_choice_){
	  if(choice.second <= threshold){
	    bit_num_ = choice.first;
	    return; 
          }
	}	
	bit_num_ = max_bit_;
   }

   template <typename T> T apply(T& v) {
     if (round_type_==RoundMethod::RANDOM) {
       return randomizedRound(v);
     } else {
       return HardRound(v);
     }
   }

   template <typename T> T HardRound(T& v) {
     auto epsilon = std::pow(2, -bit_num_);
     return (std::round(v/epsilon))*epsilon;
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
    Strategy filter_type_;
    RoundMethod round_type_;
};

} // namespace PS
