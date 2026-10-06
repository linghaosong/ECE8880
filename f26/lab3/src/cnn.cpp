#include <cmath>
#include <tapa.h>
#include "cnn.h"

// Read the input image from memory and stream it to cnncore.
void read_input(tapa::mmap<float> in_img,
                tapa::ostream<float>& in_img_stream) {
  ...
}

// Read the weights from memory and stream them to cnncore.
void read_weight(tapa::mmap<float> weight,
                 tapa::ostream<float>& weight_stream) {
  ...
}

// Read the biases from memory and stream them to cnncore.
void read_bias(tapa::mmap<float> bias,
               tapa::ostream<float>& bias_stream) {
  ...
}

// Read the output image from cnncore and write it to memory.
void write_output(tapa::istream<float>& out_img_stream,
                  tapa::mmap<float> out_img) {
  ...
}

// CNN core with stream input and output.
void cnncore(
    tapa::istream<float>& in_img_stream,
    tapa::istream<float>& weight_stream,
    tapa::istream<float>& bias_stream,
    tapa::ostream<float>& out_img_stream) {
  ...
}

void CnnKernel(
    tapa::mmap<float> in_img,
    tapa::mmap<float> weight,
    tapa::mmap<float> bias,
    tapa::mmap<float> out_img) {
  tapa::stream<float, kStreamDepth> ...;

  tapa::task()
      .invoke(read_input, ...)
      .invoke(read_weight, ...)
      ...
      .invoke(write_output, out_img_stream, out_img);
}
