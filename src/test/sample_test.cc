#include "filter/sample.h"
#include "gtest/gtest.h"

using namespace PS;

TEST(Sample, AllBasics) {
    typedef float V;

    SArray<V> arr({-0.2, 0.1, -0.35, 0.4, 0.33, -0.05});
    std::vector<int> test_K = {2,4,1,6,0,10};
    LL << "testing array:";
    LL << arr;

    for(auto& K : test_K) {
	LL << "testing K: " << K;
        SArray<V> test_arr; test_arr.copyFrom(arr);
        sampleThresholdCut(test_arr, K);
        LL << "\nsamped results";
        LL << test_arr;
    }
}

int main(int argc, char *argv[]) {
  FLAGS_logtostderr = 1;
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
