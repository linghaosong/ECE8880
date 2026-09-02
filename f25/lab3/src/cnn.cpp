#include <cmath>
#include <tapa.h>
#include "cnn.h"

/*
void read_input(tapa::mmap<float>...,
                tapa::ostream<float>...,
                const int...
                ...);

void read_weight(tapa::mmap<float>...,
                 tapa::ostream<float>...,
                 const int...
                 ...);

void read_bias(tapa::mmap<float>...,
               tapa::ostream<float>...,
               const int...
               ...);
            
void write_output(tapa::mmap<float>...,
                  tapa::istream<float>...,
                  const int...
                  ...);

void cnn_core(tapa::istream<float>...,
              ...,
              tapa::ostream<float>...
              const int...
              ...);
...
*/

void cnncore(
  tapa::mmap<float> in_img,
  tapa::mmap<float> weight,
  tapa::mmap<float> bias,
  tapa::mmap<float> out_img,
  const int kNum,
  const int kKernel,
  const int kImSize,
  const int kInImSize,
  const int kOutImSize) {
  static float C[kNum_0][kImSize_0][kImSize_0];

  for (int i = 0; i < kNum; ++i) {
  #pragma HLS loop_tripcount min=1 max=kNum_0
    for (int h = 0; h < kImSize; ++h) {
    #pragma HLS loop_tripcount min=1 max=kImSize_0
      for (int w = 0; w < kImSize; ++w) {
      #pragma HLS loop_tripcount min=1 max=kImSize_0
        C[i][h][w] = bias[i];
      }
    }
  }

  // Convolution
  for (int i = 0; i < kNum; ++i) {
  #pragma HLS loop_tripcount min=1 max=kNum_0
    for (int j = 0; j < kNum; ++j) {
    #pragma HLS loop_tripcount min=1 max=kNum_0
      for (int h = 0; h < kImSize; ++h) {
      #pragma HLS loop_tripcount min=1 max=kImSize_0
        for (int w = 0; w < kImSize; ++w) {
        #pragma HLS loop_tripcount min=1 max=kImSize_0
          for (int p = 0; p < kKernel; ++p) {
          #pragma HLS loop_tripcount min=1 max=kKernel_0
            for (int q = 0; q < kKernel; ++q) {
            #pragma HLS loop_tripcount min=1 max=kKernel_0
              C[i][h][w] += weight(i, j, p, q) * in_img(j, h + p, w + q);
            }
          }
        }
      }
    }
  }
	
	// ReLU
	for (int i = 0; i < kNum; ++i) {
  #pragma HLS loop_tripcount min=1 max=kNum_0
    for (int h = 0; h < kImSize; ++h) {
    #pragma HLS loop_tripcount min=1 max=kImSize_0
      for (int w = 0; w < kImSize; ++w) {
      #pragma HLS loop_tripcount min=1 max=kImSize_0
        C[i][h][w] = max(0.f, C[i][h][w]);
      }
    }
  }
	
	// Max pooling
  for (int i = 0; i < kNum; ++i) {
  #pragma HLS loop_tripcount min=1 max=kNum_0
    for (int h = 0; h < kOutImSize; ++h) {
    #pragma HLS loop_tripcount min=1 max=kOutImSize_0
      for (int w = 0; w < kOutImSize; ++w) {
      #pragma HLS loop_tripcount min=1 max=kOutImSize_0
        out_img(i, h, w) = max(
          max(C[i][h * 2][w * 2    ], C[i][h * 2 + 1][w * 2    ]),
          max(C[i][h * 2][w * 2 + 1], C[i][h * 2 + 1][w * 2 + 1]));
      }
    }
  }
}

void CnnKernel(
  tapa::mmap<float> in_img,
  tapa::mmap<float> weight,
  tapa::mmap<float> bias,
  tapa::mmap<float> out_img,
  const int kNum,
  const int kKernel,
  const int kImSize,
  const int kInImSize,
  const int kOutImSize) {
  tapa::task()
    .invoke(cnncore, in_img, weight,  bias, out_img, kNum, kKernel, kImSize, kInImSize, kOutImSize);
}

