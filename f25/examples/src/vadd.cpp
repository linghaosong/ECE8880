#include <cmath>
#include <tapa.h>
#include "vadd.h"

const int kVectorLen = 128;

void read_vector(
  tapa::mmap<float> input_v, 
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8192
#pragma HLS PIPELINE II=1
    q_out.write(input_v[i]);
  }
}

void write_vector(
  tapa::mmap<float> output_v, 
  tapa::istream<float> &q_in,
  const int vlen) {
  for (int i = 0; i < vlen; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8192
#pragma HLS PIPELINE II=1
    output_v[i] = q_in.read();
  }
}

//v0: cmp piepline off
void vadd_v0(
  tapa::istream<float> &q_in1,
  tapa::istream<float> &q_in2,
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
    float v1[kVectorLen];
    float v2[kVectorLen];
    float vout[kVectorLen];

rd_v1v2:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      v1[j] = q_in1.read();
      v2[j] = q_in2.read();
    }

cmp:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE off
      vout[j] = v1[j] + v2[j];
    }

wr_out:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      q_out.write(vout[j]);
    }
  }
}

//v1: cmp piepline II=1
void vadd_v1(
  tapa::istream<float> &q_in1,
  tapa::istream<float> &q_in2,
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
    float v1[kVectorLen];
    float v2[kVectorLen];
    float vout[kVectorLen];

rd_v1v2:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      v1[j] = q_in1.read();
      v2[j] = q_in2.read();
    }

cmp:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      vout[j] = v1[j] + v2[j];
    }

wr_out:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      q_out.write(vout[j]);
    }
  }
}

//v2: cmp loop, fully unrolled
void vadd_v2(
  tapa::istream<float> &q_in1,
  tapa::istream<float> &q_in2,
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
    float v1[kVectorLen];
    float v2[kVectorLen];
    float vout[kVectorLen];
#pragma HLS ARRAY_PARTITION variable=v1 complete
#pragma HLS ARRAY_PARTITION variable=v2 complete
#pragma HLS ARRAY_PARTITION variable=vout complete

rd_v1v2:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      v1[j] = q_in1.read();
      v2[j] = q_in2.read();
    }

cmp:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS UNROLL factor=kVectorLen
      vout[j] = v1[j] + v2[j];
    }

wr_out:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      q_out.write(vout[j]);
    }
  }
}

//v3: cmp loop, unrolled for a factor of 16
const int T_factor = 16;
void vadd_v3(
  tapa::istream<float> &q_in1,
  tapa::istream<float> &q_in2,
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
    float v1[kVectorLen];
    float v2[kVectorLen];
    float vout[kVectorLen];
#pragma HLS ARRAY_PARTITION variable=v1 cyclic factor=T_factor
#pragma HLS ARRAY_PARTITION variable=v2 cyclic factor=T_factor
#pragma HLS ARRAY_PARTITION variable=vout cyclic factor=T_factor

rd_v1v2:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      v1[j] = q_in1.read();
      v2[j] = q_in2.read();
    }

cmp:
    for (int j = 0; j < kVectorLen; j += T_factor) {
    #pragma HLS PIPELINE II=1
      for (int p = 0; p < T_factor; p++) {
      #pragma HLS UNROLL
        vout[j + p] = v1[j + p] + v2[j + p];
      }
    }
    // What coulb be a potential problem here? How to make this loop more robust?

wr_out:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      q_out.write(vout[j]);
    }
  }
}

//v4: parallelize add using tapa::invoke
// that is to palce parallel modules
void vadd_v4_upper(
  tapa::istream<float> &q_in,
  tapa::ostreams<float, 16> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
    float v1[kVectorLen];
#pragma HLS ARRAY_PARTITION variable=v1 cyclic factor=T_factor

rd_v1:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      v1[j] = q_in.read();
    }

sendout:
    for (int j = 0; j < kVectorLen; j += T_factor) {
    #pragma HLS PIPELINE II=1
      for (int p = 0; p < T_factor; p++) {
      #pragma HLS UNROLL
        q_out[p].write(v1[j + p]);
      }
    }
  }
}

void vadd_v4_adder(
  tapa::istream<float> &q_iop1,
  tapa::istream<float> &q_iop2,
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
  #pragma HLS LOOP_TRIPCOUNT min=1 max=64
    for (int j = 0; j < kVectorLen; j += T_factor) {
    #pragma HLS PIPELINE II=1
      q_out.write(q_iop1.read() + q_iop2.read());
    }
  }
}

void vadd_v4_lower(
  tapa::istreams<float, 16> &q_in,
  tapa::ostream<float> &q_out,
  const int vlen) {
  for (int i = 0; i < vlen; i += kVectorLen) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
    float vout[kVectorLen];
#pragma HLS ARRAY_PARTITION variable=vout cyclic factor=T_factor

rd:
    for (int j = 0; j < kVectorLen; j += T_factor) {
    #pragma HLS PIPELINE II=1
      for (int p = 0; p < T_factor; p++) {
      #pragma HLS UNROLL
        vout[j + p] = q_in[p].read();
      }
    }

wr_out:
    for (int j = 0; j < kVectorLen; j++) {
    #pragma HLS PIPELINE II=1
      q_out.write(vout[j]);
    }
  }
}

/*
void VaddKernel(
  tapa::mmap<float> input_v1,
  tapa::mmap<float> input_v2,
  tapa::mmap<float> output_v,
  const int vlen
) {
  //tapa::task().invoke(vadd, input_v1, input_v2, output_v);
  tapa::stream<float, 2> q_v1("q_v1");
  tapa::stream<float, 2> q_v2("q_v1");
  tapa::stream<float, 2> q_out("q_out");

  tapa::task()
              .invoke(read_vector, input_v1, q_v1, vlen)
              .invoke(read_vector, input_v2, q_v2, vlen)
              //.invoke(vadd_v0, q_v1, q_v2, q_out, vlen)
              //.invoke(vadd_v1, q_v1, q_v2, q_out, vlen)
              //.invoke(vadd_v2, q_v1, q_v2, q_out, vlen)
              .invoke(vadd_v3, q_v1, q_v2, q_out, vlen)
              .invoke(write_vector, output_v, q_out, vlen);
}
*/

// V4
void VaddKernel(
  tapa::mmap<float> input_v1,
  tapa::mmap<float> input_v2,
  tapa::mmap<float> output_v,
  const int vlen
) {
  //tapa::task().invoke(vadd, input_v1, input_v2, output_v);
  tapa::stream<float, 2> q_v1("q_v1");
  tapa::stream<float, 2> q_v2("q_v1");
  tapa::streams<float, T_factor, 2> q_op1("q_op1");
  tapa::streams<float, T_factor, 2> q_op2("q_op2");
  tapa::streams<float, T_factor, 2> q_add("q_add");

  tapa::stream<float, 2> q_out("q_out");

  tapa::task()
              .invoke(read_vector, input_v1, q_v1, vlen)
              .invoke(read_vector, input_v2, q_v2, vlen)
              .invoke(vadd_v4_upper, q_v1, q_op1, vlen)
              .invoke(vadd_v4_upper, q_v2, q_op2, vlen)
              .invoke<tapa::join, T_factor>(vadd_v4_adder, q_op1, q_op2, q_add, vlen)
              .invoke(vadd_v4_lower, q_add, q_out, vlen)
              .invoke(write_vector, output_v, q_out, vlen);
}
