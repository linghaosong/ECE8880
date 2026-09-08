#include <chrono>
#include <iostream>
#include <string>

#include "dot_product.h"

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::clog;
using std::endl;
using std::string;

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(btstm, "", "path to the bitstream file, run csim if empty");

// Dot product of the input vector with itself on host for result verification
void DotProduct_host(
    aligned_vector<float_v16> & input_v,
    aligned_vector<float> & output_sum) {
    float sum = 0.0f;
    for (int i = 0; i < kNumVecs; i++) {
        for (int j = 0; j < kVecWidth; j++) {
            sum += input_v[i][j] * input_v[i][j];
        }
    }
    output_sum[0] = sum;
}

void InitializeData(
    aligned_vector<float_v16> & input_v) {
    for (int i = 0; i < kNumVecs; i++) {
        for (int j = 0; j < kVecWidth; j++) {
            input_v[i][j] = 1.0 * ((i * kVecWidth + j) % 64) / 64;
        }
    }
}

bool IsError(float a, float b) {
  return fabs((a - b) / (a + b)) > 1e-3f && fabs(a - b) > 0.05f;
}

int Verify(aligned_vector<float> & output_device,
           aligned_vector<float> & output_host) {
    int error = 0;
    if (IsError(output_device[0], output_host[0])) {
        std::cout << "Mismatch: device " << output_device[0]
                  << ", host " << output_host[0] << std::endl;
        error++;
    }
    return error;
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);
  //host data
  aligned_vector<float_v16> v(kNumVecs);
  aligned_vector<float> sum_dev(1, 0.0);
  aligned_vector<float> sum_host(1, 0.0);
  aligned_vector<uint32_t> cycle_count(1, 0);

  InitializeData(v);

  DotProduct_host(v, sum_host);

  //invoke kernel
  tapa::invoke(DotProductKernel, FLAGS_btstm,
                 tapa::read_only_mmap<float_v16>(v),
                 tapa::write_only_mmap<float>(sum_dev),
                 tapa::write_only_mmap<uint32_t>(cycle_count));

  clog << "Kernel cycle count: " << cycle_count[0]
       << " (only meaningful in cosim and on board)" << endl;

  //verify
  int error = Verify(sum_dev, sum_host);
  if (error != 0) {
    clog << "Found " << error << " error" << (error > 1 ? "s\n" : "\n");
    clog << "FAIL" << endl;
    return EXIT_FAILURE;
  } else {
    clog << "PASS" << endl;
    return EXIT_SUCCESS;
  }
}
