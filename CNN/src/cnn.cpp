#include <cmath>
#include <tapa.h>
#include "cnn.h"

void CnnKernel(
  tapa::mmap<float> input,
  tapa::mmap<float> weight,
  tapa::mmap<float> bias,
  tapa::mmap<float> output) {

  static float C[kNum][kImSize][kImSize];

    for (int i = 0; i < kNum; ++i) {
      for (int h = 0; h < kImSize; ++h) {
        for (int w = 0; w < kImSize; ++w) {
          C[i][h][w] = bias[i];
        }
      }
    }

    // Convolution
    for (int i = 0; i < kNum; ++i) {
      for (int j = 0; j < kNum; ++j) {
        for (int h = 0; h < kImSize; ++h) {
          for (int w = 0; w < kImSize; ++w) {
            for (int p = 0; p < kKernel; ++p) {
              for (int q = 0; q < kKernel; ++q)
                C[i][h][w] += weight(i, j, p, q) * input(j, h + p, w + q);
              }
            }
        }
      }
    }
	
	// ReLU
	for (int i = 0; i < kNum; ++i) {
      for (int h = 0; h < kImSize; ++h) {
        for (int w = 0; w < kImSize; ++w) {
          C[i][h][w] = max(0.f, C[i][h][w]);
        }
      }
    }
	
	// Max pooling
    for (int i = 0; i < kNum; ++i) {
      for (int h = 0; h < kOutImSize; ++h) {
        for (int w = 0; w < kOutImSize; ++w) {
          output(i, h, w) = max(
            max(C[i][h * 2][w * 2    ], C[i][h * 2 + 1][w * 2    ]),
            max(C[i][h * 2][w * 2 + 1], C[i][h * 2 + 1][w * 2 + 1]));
        }
      }
    }
  }

