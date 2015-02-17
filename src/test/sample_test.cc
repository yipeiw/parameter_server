#include "filter/weight_sensitive_sample.h"
#include "gtest/gtest.h"
#include <bitset>
#include <fstream>

using namespace PS;

TEST(Sample, AllBasics) {
    typedef float V;
    int arrNum = 50;
    SArray<V> arr(arrNum, 0.0);
    int seed = 5323;
    for (int i=0; i < arrNum; ++i) {
      seed = (seed * 8253729 + 2396403) % 32767;
      int sign = (seed % 2) >0 ? 1:-1;
      arr[i] = (seed % 100)*sign;
    }

    std::ofstream datafile; datafile.open("arr.log");
    for (int i=0; i < arrNum; ++i) {
      datafile << arr[i] << "\n";
    }
    datafile.close();

    int testK[] = {10,20,30,40};

    SArray<V> test_arr;
    for(int i=0; i<4; i++) {
        int K = testK[i];
	LL << "testing K: " << K;
        test_arr.resize(arrNum, 0);
        LL << "test arr prepare "<<arrNum;
	for(int i=0; i<arrNum; i++) { test_arr[i] = fabs(arr[i]); }
        LL << "start finding ...";

	V threshold1 = findKthLargest(test_arr, 0, arrNum-1, K);
        LL << "exact Kth " << threshold1;
        V threshold2 = ApproximateKth(test_arr, K);
        LL << "approx Kth " << threshold2;
        //WeightSensitiveSample(test_arr, K);
        //LL << "\nsamped results";
        //LL << test_arr;
    }
}

/*TEST(Round, Int16) {
    RoundingFilter rf;
    rf.set_bit(5);
    LL << "double type " <<typeid(double).name();
    std::vector<double> arr = {2.13245, 0.324, 10.32, 0.65, -0.65};
    for(auto element : arr) {
      for (int i=0; i<5; i++) {
        auto ele = rf.randomizedRound(element);
        LL << ele <<"; " << std::bitset<16>(ele) << " size " <<sizeof(ele);
      }
    }
}*/

int main(int argc, char *argv[]) {
  FLAGS_logtostderr = 1;
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
