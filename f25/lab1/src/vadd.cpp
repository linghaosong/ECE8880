#include <cmath>
#include <tapa.h>
#include "vadd.h"

void vadd(
  tapa::mmap<float> input_v1,
  tapa::mmap<float> input_v2,
  tapa::mmap<float> output_v) {
    for (int i = 0; i < kVectorLen; i++) {
    #pragma HLS PIPELINE OFF
      output_v[i] = input_v1[i] + input_v2[i];
    }
}

void VaddKernel(
  tapa::mmap<float> input_v1,
  tapa::mmap<float> input_v2,
  tapa::mmap<float> output_v
) {
  tapa::task().invoke(vadd, input_v1, input_v2, output_v);
}

