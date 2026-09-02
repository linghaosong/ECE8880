#include <chrono>
#include <iostream>
#include <string>

#include "cnn.h"

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::clog;
using std::endl;
using std::string;

DEFINE_string(btstm, "", "path to the bitstream file, run csim if empty");
DEFINE_string(dtf, "./data", "data directory, default is ./data");

int main(int argc, char** argv) {
  //host data
  aligned_vector<float> h_input(kNum * kInImSize * kInImSize);
  aligned_vector<float> h_weight(kNum * kNum * kKernel * kKernel);
  aligned_vector<float> h_bias(kNum);
  aligned_vector<float> h_output(kNum * kOutImSize * kOutImSize);

  //a vector on host to store data from FPGA device
  aligned_vector<float> d_output(kNum * kOutImSize * kOutImSize);

  if (argc > 2) {
    clog << "Usage: " << argv[0] << " [data dir]\n";
    return EXIT_FAILURE;
  }

  LoadData(FLAGS_dtf, h_input, h_weight, h_bias);

  clog << "CNN computation on CPU using CnnSequential\n";
  const auto begin = steady_clock::now();
  CnnSequential(h_input, h_weight, h_bias, h_output);
  const auto end = steady_clock::now();

  uint64_t run_time_us = duration_cast<microseconds>(end - begin).count();
  float gflops = float(kNum) * kNum * kImSize * kImSize * kKernel * kKernel * 2
                   / (run_time_us * 1e3);
  clog << "Time: " << run_time_us * 1e-6 << " s\n";
  clog << "Perf: " << gflops << " GFlops (don't trust if you sw emu hw emu)\n";
  
  //run tapa kernel: 
  //FLAGS_btstm = '', -> software simulation
  //FLAGS_btstm = ***.hw_emu.xclbin, -> hardware emulation
  //FLAGS_btstm = ***.xclbin, -> FPGA execution
  double time_taken
    = tapa::invoke(CnnKernel, FLAGS_btstm,
                   tapa::read_only_mmap<float>(h_input), 
                   tapa::read_only_mmap<float>(h_weight), 
                   tapa::read_only_mmap<float>(h_bias), 
                   tapa::write_only_mmap<float>(d_output));
  time_taken *= 1e-6; // total time in mini second
  printf("Kernel time is %f ms\n", time_taken*1000);

  int error = Verify(FLAGS_dtf, d_output);
  if (error != 0) {
    clog << "Found " << error << " error" << (error > 1 ? "s\n" : "\n");
    clog << "FAIL" << endl;
    return EXIT_FAILURE;
  } else {
    clog << "PASS" << endl;
    return EXIT_SUCCESS;
  }
}
