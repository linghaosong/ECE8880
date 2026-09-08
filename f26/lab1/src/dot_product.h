#ifndef DOT_PRODUCT_H_
#define DOT_PRODUCT_H_

#include <hls_vector.h>
#include <tapa.h>

constexpr int kVectorLen = 8192;
constexpr int kVecWidth = 16;
constexpr int kNumVecs = kVectorLen / kVecWidth;

using float_v16 = hls::vector<float, kVecWidth>;

void DotProductKernel(
    tapa::mmap<float_v16> input_v,
    tapa::mmap<float> output_sum,
    tapa::mmap<uint32_t> cycle_count);

#endif
