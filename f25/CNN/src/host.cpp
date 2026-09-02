#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <tapa.h>
#include "cnn.h"

using std::clog;
using std::endl;
using std::string;

// Sequential CNN implementation
void CnnSequential(
    aligned_vector<float> & input,
    aligned_vector<float> & weight, 
    aligned_vector<float> & bias,
    aligned_vector<float> & output) {

  // Allocate memory on heap to avoid stack overflow.
  static float C[kNum][kImSize][kImSize];

  for (int i = 0; i < kNum; ++i) {
    for (int h = 0; h < kImSize; ++h) {
      for (int w = 0; w < kImSize; ++w)
        C[i][h][w] = bias[i];
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

void LoadData(const string& data_dir, 
               aligned_vector<float> & input,
               aligned_vector<float> & weight, 
               aligned_vector<float> & bias) {
  const char kInputFile[] = "/input.bin";
  const char kWeightFile[] = "/weight.bin";
  const char kBiasFile[] = "/bias.bin";

  int input_fd = open((data_dir + kInputFile).c_str(), O_RDONLY);
  int weight_fd = open((data_dir + kWeightFile).c_str(), O_RDONLY);
  int bias_fd = open((data_dir + kBiasFile).c_str(), O_RDONLY);

  if (input_fd == -1) {
    clog << "Cannot find " << kInputFile << endl;
    exit(EXIT_FAILURE);
  }
  if (weight_fd == -1) {
    clog << "Cannot find " << kWeightFile << endl;
    exit(EXIT_FAILURE);
  }
  if (bias_fd == -1) {
    clog << "Cannot find " << kBiasFile << endl;
    exit(EXIT_FAILURE);
  }

  auto input_in = reinterpret_cast<float*>(mmap(
      nullptr, sizeof(*input.data()) * kNum * kInImSize * kInImSize, PROT_READ, MAP_SHARED, input_fd, 0));
  if (input_in == MAP_FAILED) {
    clog << "Incomplete " << kInputFile << endl;
    close(input_fd);
    exit(EXIT_FAILURE);
  }

  auto weight_in = reinterpret_cast<float*>(mmap(
      nullptr, sizeof(*weight.data()) * kNum * kNum * kKernel * kKernel, PROT_READ, MAP_SHARED, weight_fd, 0));
  if (weight_in == MAP_FAILED) {
    clog << "Incomplete " << kWeightFile << endl;
    close(weight_fd);
    exit(EXIT_FAILURE);
  }

  float* bias_in = reinterpret_cast<float*>(mmap(
      nullptr, sizeof(*bias.data()) * kNum, PROT_READ, MAP_SHARED, bias_fd, 0));
  if (bias_in == MAP_FAILED) {
    clog << "Incomplete " << kBiasFile << endl;
    close(bias_fd);
    exit(EXIT_FAILURE);
  }

  memcpy(input.data(), input_in, sizeof(*input.data()) * kNum * kInImSize * kInImSize);
  memcpy(weight.data(), weight_in, sizeof(*weight.data()) * kNum * kNum * kKernel * kKernel);
  memcpy(bias.data(), bias_in,  sizeof(*bias.data()) * kNum);
  munmap(input_in, sizeof(*input.data()) * kNum * kInImSize * kInImSize);
  munmap(weight_in, sizeof(*weight.data()) * kNum * kNum * kKernel * kKernel);
  munmap(bias_in,  sizeof(*bias.data()) * kNum);
  close(input_fd);
  close(weight_fd);
  close(bias_fd);
}

float IsError(float a, float b) {
  return fabs((a - b) / (a + b)) > 1e-3f && fabs(a - b) > 0.05f;
}

int Verify(const string& data_dir,
           aligned_vector<float> & output) {
  int error = 0;
  const char kOutputFile[] = "/output.bin";
  int fd = open((data_dir + kOutputFile).c_str(), O_RDONLY);
  if (fd == -1) {
    clog << "Cannot find " << kOutputFile << endl;
    return EXIT_FAILURE;
  }
  auto ground_truth = reinterpret_cast<float*>(mmap(
      nullptr, sizeof(*output.data()) * output.size(), PROT_READ, MAP_SHARED, fd, 0));
  if (ground_truth == MAP_FAILED) {
    clog << "Incomplete " << kOutputFile << endl;
    close(fd);
    return EXIT_FAILURE;
  }
  bool first = true;
  for (int i = 0; i < kNum; ++i) {
    for (int h = 0; h < kOutImSize; ++h) {
      for (int w = 0; w < kOutImSize; ++w) {
        if (IsError(output(i, h, w), ground_truth[(i) * kOutImSize * kOutImSize + (h) * kOutImSize + (w)])) {
          if (first) {
            clog << "First error: get " << output(i, h, w) << ", expecting "
                 << ground_truth[(i) * kOutImSize * kOutImSize + (h) * kOutImSize + (w)] << " @ i = " << i << ", h = " << h
                 << ", w = " << w << endl;
            first = false;
          }
          ++error;
        }
      }
    }
  }
  munmap(ground_truth, sizeof(*output.data()) * output.size());
  close(fd);
  return error;
}
