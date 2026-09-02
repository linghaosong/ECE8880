#include <chrono>
#include <iostream>
#include <string>

#include "vadd.h"

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::clog;
using std::endl;
using std::string;

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(btstm, "", "path to the bitstream file, run csim if empty");
DEFINE_int32(len, 8192, "length of vectors");

// Vector addition on host for result verification
void Vadd_host(
    aligned_vector<float> & input_v1,
    aligned_vector<float> & input_v2, 
    aligned_vector<float> & output_v,
    const int vlen) {
    for (int i = 0; i < vlen; i++) {
        output_v[i] = input_v1[i] + input_v2[i];
    }
}

void InitializeData(
    aligned_vector<float> & input_v1,
    aligned_vector<float> & input_v2,
    const int vlen) {
    for (int i = 0; i < vlen; i++) {
        input_v1[i] = 1.0 * i;
        input_v2[i] = 2.0 * i;
    }
}

bool IsError(float a, float b) {
  return fabs((a - b) / (a + b)) > 1e-3f && fabs(a - b) > 0.05f;
}

int Verify(aligned_vector<float> & output_device,
           aligned_vector<float> & output_host,
           const int vlen) {
    int error = 0;
    for (int i = 0; i < vlen; i++) {
        if (IsError(output_device[i], output_host[i])) {
            if (error < 10) {
                std::cout << "Mismatch at index " << i << ": device "
                          << output_device[i] << ", host " << output_host[i] << std::endl;
            }
            error++;
           }
    }
    return error;
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);
  //host data
  aligned_vector<float> v1(FLAGS_len);
  aligned_vector<float> v2(FLAGS_len);
  aligned_vector<float> v_result_dev(FLAGS_len, 0.0);
  aligned_vector<float> v_result_host(FLAGS_len, 0.0);

  InitializeData(v1, v2, FLAGS_len);

  Vadd_host(v1, v2, v_result_host, FLAGS_len);

  //invoke kernel
  tapa::invoke(VaddKernel, FLAGS_btstm,
                  tapa::read_only_mmap<float>(v1),
                  tapa::read_only_mmap<float>(v2),
                  tapa::write_only_mmap<float>(v_result_dev),
                  FLAGS_len);
  
  //verify
  int error = Verify(v_result_dev, v_result_host, FLAGS_len);
  if (error != 0) {
    clog << "Found " << error << " error" << (error > 1 ? "s\n" : "\n");
    clog << "FAIL" << endl;
    return EXIT_FAILURE;
  } else {
    clog << "PASS" << endl;
    return EXIT_SUCCESS;
  }
}
