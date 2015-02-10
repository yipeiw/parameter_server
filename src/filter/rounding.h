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
     bit_num_ = 3;
     std::vector<int> bit_num_ = {3,11};
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
 
   void set_bit(int bit) {  bit_num_ = bit; }
   int get_bit() {  return bit_num_;  }
  
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

   template <typename T>
   void apply(SArray<T> Val, SArray<int16_t> Filter_Val) {
     if (round_type_==RoundMethod::RANDOM) {
       for (int i=0; i < Val.size(); i++) {
         Filter_Val[i] = randomizedRound(Val[i]);
       }
     } else { //to do
       return;
     }
   }
   template <typename T, typename V>
   void decode(SArray<V> Filter_Val, SArray<T>& Val) {
     for(int i=0; i < Filter_Val.size(); i++) {
       Val[i] = Filter_Val[i]*std::pow(2, -bit_num_);
     }
   }

   template <typename T> int16_t HardRound(T& v) {
     auto epsilon = std::pow(2, -bit_num_);
     return static_cast<int16_t> (std::round(v/epsilon));
   }

   template <typename T> int16_t randomizedRound(T& v) {
    auto epsilon = std::pow(2, -bit_num_);
    
    //auto upper = (std::ceil(v/epsilon))*epsilon;
    //auto lower = (std::floor(v/epsilon))*epsilon;
    //auto threshold = v-lower;
    auto upper = static_cast<int16_t>(std::ceil(v/epsilon));
    auto lower = static_cast<int16_t>(std::floor(v/epsilon));
    auto threshold = v/epsilon - lower;
    //LL<<"up:"<<upper<<"; lo:"<<lower << ", threshold " << threshold;
    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    auto randNum = distribution(generator);

    //LL<<"rand "<<randNum;
    if (randNum < threshold) {
      return upper;
    } else {
      return lower;
    }
   }
  
  private:
    int bit_num_;
    std::map<int, double> bit_choice_;
    const int max_bit_ = 11; //should be max(bit_choice)
    Strategy filter_type_;
    RoundMethod round_type_;
};

} // namespace PS
