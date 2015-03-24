#pragma once
#include "filter/filter.h"
#include <math.h>

namespace PS {

class PrecisionFilter : public Filter {
 public:
  PrecisionFilter() {
   memcpy(&double_v_, &kuint64max, sizeof(double));
   memcpy(&float_v_, &kuint32max, sizeof(float));
  }

  void encode(const MessagePtr& msg) {
    //LL<<"precision encodefunc "<<msg->value.size();
    convert(msg, true);
  }
 
  void decode(const MessagePtr& msg) {
    //LL<<"precision decodefunc "<<msg->value.size();
    convert(msg, false);
  }

 private:
  float float_v_;
  double double_v_;

  bool marked(float v) {return v!=v;}
  bool marked(double v) {return v!=v;}
  void mark(float* v) { *v = float_v_; }
  void mark(double* v) { *v = double_v_; }

  void convert(const MessagePtr& msg, bool encode) {
    auto filter_conf = CHECK_NOTNULL(find(FilterConfig::PRECISION, msg));   
    int n = filter_conf->precision_filter_size();
   
    //LL << "encode "<<encode<<" converting msg:"<<msg->value.size(); 
    //CHECK_EQ(msg->value.size(), msg->task.value_type_size());

    //CHECK_GT(msg->task.value_type_size()+1, n);
    int N = std::min(n, static_cast<int>(msg->value.size()));

    for (int i = 0; i < N; i++) {
      auto precision_conf = filter_conf->mutable_precision_filter(i);
      int idx = precision_conf->id();
      CHECK_GT(msg->value.size(), idx);
      //LL << "precision filter arr "<< idx;
      auto type = msg->task.value_type(idx);
      if (type == DataType::FLOAT) {
        if (encode) {
          msg->value[idx] = EncodeConvert<float>(msg->value[idx], precision_conf);
        } else {
          msg->value[idx] = DecodeConvert<float>(msg->value[idx], precision_conf);
        }
      } else if (type == DataType::DOUBLE) {
        if (encode) {
          msg->value[idx] = EncodeConvert<double>(msg->value[idx], precision_conf);
        } else {
          msg->value[idx] = DecodeConvert<double>(msg->value[idx], precision_conf);
        }
      } else {LL << "error type "<< type;}
    }
    //LL << "encode "<<encode<<" finish converting msg:"<<msg->value.size();

  }

  template <typename V>
  SArray<char> EncodeConvert(const SArray<char>& array, FilterConfig::PrecisionConfig* conf) {
    SArray<V> orig(array);
    //LL << "orig size "<<orig.size();
    SArray<int8> code(orig.size()); //store exponent in 1 byte
    int8* code_ptr = code.data();

    int exponent, up_bound=30, lo_bound=-30;
    V mantissa;
    for (int i=0; i < orig.size(); i++) {
      if (marked(orig[i])) {
        *(code_ptr++) = static_cast<int8>(0xFF);
        //LL << "mark detected "<<i;
      } else {
        mantissa = frexp(orig[i], &exponent);
        bool sign = mantissa < 0;
        //LL <<" origin "<<orig[i]<<" mantissa "<<mantissa<<" exponent "<<exponent;
        exponent = exponent > up_bound ? up_bound : (exponent < lo_bound ? lo_bound: exponent);
        *(code_ptr++) = static_cast<int8>( (exponent<<1) +sign );     
      }
    }

    return SArray<char>(code);
  }

  template <typename V>
  SArray<char> DecodeConvert(const SArray<char>& array, FilterConfig::PrecisionConfig* conf) {
    int8 markNum = static_cast<int8>(0xFF);
    //LL << "arr size "<<array.size();

    int8* code_ptr = SArray<int8>(array).data();
    SArray<V> orig(array.size());

    for (int i = 0; i < orig.size(); i++) {
      int8 r = *(code_ptr++);
      if (r==markNum) {
        mark(&orig[i]);
        //LL<<"decode "<<r<< " marked";
      } else {
        double sign = (r & 0x1) ? -1:1;
        orig[i] = ldexp(sign, r >> 1); //only keep exponent
        //LL << "sign mantissa "<<sign<<" decode "<< (r>>1)<<" to "<<orig[i];
      }
    }
    return SArray<char>(orig); 
  }

};

}
