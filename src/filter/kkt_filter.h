#pragma once
#include "filter/sparse_filter.h"
#include "filter/sample.h"
#include "linear_method/linear_method.pb.h"
#include "base/bitmap.h"
#include "parameter/kv_buffered_vector.h"

#include "math.h"

namespace PS{

class KKTFilter : public SparseFilter{
  public:
    double violation_; //only useful for server, public just for convenience
    void init(KVBufferedVectorPtr<Key, double> model_, std::vector<int>& fea_grp_) {
      for (int grp : fea_grp_) {  
        size_t n = model_->key(grp).size(); 
        active_set_[grp].resize(n, true);
      }
    }

    void reset(std::vector<int>& fea_grp_) {
      for (int grp : fea_grp_) active_set_[grp].fill(true);
    }

    bool test(int grp, int k) {
      return active_set_[grp].test(k);
    }

    void clear(int grp, int k) {
      active_set_[grp].clear(k);
    }

    int get_nnz(int grp) {
      return active_set_[grp].nnz();
    }

    double get_threshold() { return kkt_filter_threshold_; }

    void set_threshold(double threshold) { kkt_filter_threshold_ = threshold; }

    void set_ratio(double ratio) {ratio_ = ratio; }

    double get_ratio() { return ratio_; }

    void update_threshold(double vio, int exampleNum) {
      kkt_filter_threshold_ = vio / (double)exampleNum * ratio_;
      //LL << "update kkt " << kkt_filter_threshold_ << " vio:" << vio;
    }

    //bool ApplyFilter() {}

  private:
    //the vio_ is currently hard coded in progress, should be actually tracked by the filter itself (on scheduler)
    std::unordered_map<int, Bitmap> active_set_;
    double kkt_filter_threshold_ = 1e20;
    double ratio_;
};

}
