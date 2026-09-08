#include <tapa.h>

#include "dot_product.h"

void ReadFromMmap(tapa::mmap<float_v16> input_v,
                  tapa::ostream<float_v16>& output_stream) {
  for (int i = 0; i < kNumVecs; ++i) {
    output_stream.write(input_v[i]);
  }
}

void DotProduct(tapa::istream<float_v16>& input_stream,
                tapa::ostream<float>& output_stream,
                tapa::ostream<bool>& done_stream) {
  float sum = 0.0f;
  for (int i = 0; i < kNumVecs; ++i) {
    const float_v16 v = input_stream.read();
    for (int j = 0; j < kVecWidth; ++j) {
#pragma HLS PIPELINE OFF
      sum += v[j] * v[j];
    }
  }
  output_stream.write(sum);
  done_stream.write(true);
}

void WriteToMmap(tapa::istream<float>& input_stream,
                 tapa::mmap<float> output_sum) {
  output_sum[0] = input_stream.read();
}

void Timer(tapa::istream<bool>& done_stream,
           tapa::mmap<uint32_t> cycle_count) {
  uint32_t count = 0;
  while (done_stream.empty()) {
#pragma HLS PIPELINE II=1
    ++count;
  }
  done_stream.read();
  cycle_count[0] = count;
}

void DotProductKernel(tapa::mmap<float_v16> input_v,
                      tapa::mmap<float> output_sum,
                      tapa::mmap<uint32_t> cycle_count) {
  tapa::stream<float_v16> input_stream("input_stream");
  tapa::stream<float> sum_stream("sum_stream");
  tapa::stream<bool> done_stream("done_stream");

  tapa::task()
      .invoke(ReadFromMmap, input_v, input_stream)
      .invoke(DotProduct, input_stream, sum_stream, done_stream)
      .invoke(WriteToMmap, sum_stream, output_sum)
      .invoke(Timer, done_stream, cycle_count);
}
