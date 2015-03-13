#pragma once
#include "filter/filter.h"

namespace PS {

class FixingFloatFilter : public Filter {
 public:
  FixingFloatFilter() {
    memcpy(&double_v_, &kuint64max, sizeof(double));
    memcpy(&float_v_, &kuint32max, sizeof(float));
  }

  void encode(const MessagePtr& msg) {
    convert(msg, true);
  }

  void decode(const MessagePtr& msg) {
    convert(msg, false);
  }

 private:
  // a fast random function
  float float_v_;
  double double_v_;

  bool marked(float v) {return v!=v;}
  bool marked(double v) {return v!=v;}
  void mark(float* v) { *v = float_v_; }
  void mark(double* v) { *v = double_v_; }

  static bool boolrand(int* seed) {
    *seed = (214013 * *seed + 2531011);
    return ((*seed >> 16) & 0x1) == 0;
  }

  static bool randombool(int *seed, double prob) {
    *seed = (214013 * *seed + 2531011);
    uint8_t randNum = (*seed & 0xFF);
    return ( double(randNum)/512 < prob );
  }

  // decode / encode a message
  void convert(const MessagePtr& msg, bool encode) {
    auto filter_conf = CHECK_NOTNULL(find(FilterConfig::FIXING_FLOAT, msg));
    int n = msg->value.size();
    //CHECK_EQ(n, msg->task.value_type_size());
    int k = 0;
    //LL<<"msg values "<<msg->value.size()<<" roundings "<<filter_conf->fixed_point_size();
    int N = std::min(n, filter_conf->fixed_point_size());
    
    for (int i = 0; i < N; ++i) {
      auto type = msg->task.value_type(i);
      if (type == DataType::FLOAT) {
        CHECK_GT(filter_conf->fixed_point_size(), k);
        msg->value[i] = convert<float>(msg->value[i], encode, filter_conf->mutable_fixed_point(k++));
      }
      if (type == DataType::DOUBLE) {
        CHECK_GT(filter_conf->fixed_point_size(), k);
        msg->value[i] = convert<double>(msg->value[i], encode, filter_conf->mutable_fixed_point(k++));
      }
    }
  }

  // decode / encode an array
  template <typename V>
  SArray<char> convert(const SArray<char>& array, bool encode, FilterConfig::FixedFloatConfig* conf) {
    int nbytes = conf->num_bytes();
    CHECK_GT(nbytes, 0);
    CHECK_LT(nbytes, 8);
    double ratio = static_cast<double>(1 << (nbytes*8)) - 2;
    double max_v=0, min_v=0, bin=0;
    
    if (encode && conf->rescale()) {
      if (!conf->has_min_value()) {
        conf->set_min_value(SArray<V>(array).eigenArray().minCoeff());
      }
      if (!conf->has_max_value()) {
        conf->set_max_value(SArray<V>(array).eigenArray().maxCoeff()); // to avoid max_v == min_v
      }
    }

    if (conf->rescale()) { 
      CHECK(conf->has_min_value());
      min_v = static_cast<double>(conf->min_value());
      CHECK(conf->has_max_value());
      max_v = static_cast<double>(conf->max_value());
      bin = max_v - min_v;
      if (bin==0) {conf->set_single(true); LL << "unique value encoding";}
    }

    //LL <<conf->num_bytes()<<" encode "<<encode<<" unique:"<<conf->single()<<",rescale:"<<conf->rescale()<<",max:"<<max_v<<",min:"<<min_v;

    if (conf->rescale()){
    if (encode) {
      // float/double to nbytes*8 int
      SArray<V> orig(array);
      SArray<uint8> code(orig.size() * nbytes);
      uint8* code_ptr = code.data();
      int seed = time(NULL);
      for (int i = 0; i < orig.size(); ++i) {
	uint64 r;
	if (marked(orig[i])) {
          //LL<<"detect mark in encoding";
	  r = kuint64max;
	} 
        else if (orig[i]==0) {
          r = 0;
        } else {
          if (conf->single()) {
           r = 1; 
          }
          else {
            double proj = orig[i] > max_v ? max_v : orig[i] < min_v ? min_v : orig[i];
            double tmp = (proj - min_v) / bin * ratio;
        //uint64 r = static_cast<uint64>(floor(tmp)) + boolrand(&seed);
            auto low = floor(tmp);
            double prob = tmp - low;
            r = static_cast<uint64>(low) + randombool(&seed, prob);
            r = r>0 ? r:r+1;
          }
        }
        for (int j = 0; j < nbytes; ++j) {
          *(code_ptr++) = static_cast<uint8>(r & 0xFF);
          r = r >> 8;
        }
      }
      return SArray<char>(code);
    } else {
      // nbytes*8 int to float/double
      double markNum = 0;
      for (int i=0; i < nbytes; ++i) {
        markNum += static_cast<uint64>( 0xFF << 8 * i );
      }
      //LL <<"mark num:"<< markNum;

      uint8* code_ptr = SArray<uint8>(array).data();
      SArray<V> orig(array.size() / nbytes);
      for (int i = 0; i < orig.size(); ++i) {
        double r = 0;
        for (int j = 0; j < nbytes; ++j) {
          r += static_cast<uint64>(*(code_ptr++)) << 8 * j;
        }
	if (r==markNum) {
          //LL << "detect mark in decoding"<<r;
          mark(&orig[i]);
        } else if (conf->single() && r>0){
          orig[i] = static_cast<V>(max_v); //either 0 or the unique value
        } else if (r==0) {
          orig[i] = 0;
        } else {
          orig[i] = static_cast<V>(r / ratio * bin + min_v);
        }
      }
      return SArray<char>(orig);
    }
    } else {
      if (encode) {
        SArray<V> orig(array);
        SArray<uint8> code(orig.size() * nbytes);
        uint8* code_ptr = code.data();
        //int seed = time(NULL);
        for (int i = 0; i < orig.size(); ++i) {
          uint64 r=0;
          if (marked(orig[i])) {
            //LL<<"mark in encoding";
            r = kuint64max;
          }
          for (int j = 0; j < nbytes; ++j) {
            *(code_ptr++) = static_cast<uint8>(r & 0xFF);
            r = r >> 8;
          }
         }
         return SArray<char>(code);
       } else {
         double markNum = 0;
         for (int i=0; i < nbytes; ++i) {
           markNum += static_cast<uint64>(0xFF << 8 * i);
         }

         uint8* code_ptr = SArray<uint8>(array).data();
         SArray<V> orig(array.size() / nbytes);
         for (int i = 0; i < orig.size(); ++i) {
            double r = 0;
            for (int j = 0; j < nbytes; ++j) {
              r += static_cast<uint64>(*(code_ptr++)) << 8 * j;
            }
            if (r==markNum) {
              mark(&orig[i]);
            } else {orig[i]=0;}
         }
         return SArray<char>(orig);
       }
     }
    }
};

} // namespace PS
