#include <tapa.h>

#include "vadd.h"

void ReadFromMmap(tapa::mmap<float> input_v,
                  tapa::ostream<float>& output_stream) {
  for (int i = 0; i < kVectorLen; ++i) {
    output_stream.write(input_v[i]);
  }
}

void Vadd(tapa::istream<float>& input_stream_1,
          tapa::istream<float>& input_stream_2,
          tapa::ostream<float>& output_stream) {
  for (int i = 0; i < kVectorLen; ++i) {
    const float input_1 = input_stream_1.read();
    const float input_2 = input_stream_2.read();
    // TODO: Replace 0.0 with the sum of input_1 and input_2.
    const float output = 0.0;
    output_stream.write(output);
  }
}

void WriteToMmap(tapa::istream<float>& input_stream,
                 tapa::mmap<float> output_v) {
  for (int i = 0; i < kVectorLen; ++i) {
    output_v[i] = input_stream.read();
  }
}

void VaddKernel(tapa::mmap<float> input_v1, tapa::mmap<float> input_v2,
                tapa::mmap<float> output_v) {
  tapa::stream<float> input_stream_1("input_stream_1");
  tapa::stream<float> input_stream_2("input_stream_2");
  tapa::stream<float> output_stream("output_stream");

  tapa::task()
      .invoke(ReadFromMmap, input_v1, input_stream_1)
      .invoke(ReadFromMmap, input_v2, input_stream_2)
      .invoke(Vadd, input_stream_1, input_stream_2, output_stream)
      .invoke(WriteToMmap, output_stream, output_v);
}
