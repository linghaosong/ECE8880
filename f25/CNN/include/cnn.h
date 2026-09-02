#ifndef CNN_H_
#define CNN_H_

#include <string>
#include <tapa.h>

using std::string;

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

#define weight(i, j, p, q) \
    weight[(i) * kNum * kKernel * kKernel + (j) * kKernel * kKernel + \
    (p) * kKernel + (q)]
#define input(j, h, w) \
    input[(j) * kInImSize * kInImSize + (h) * kInImSize + (w)]
#define output(i, h, w) \
    output[(i) * kOutImSize * kOutImSize + (h) * kOutImSize + (w)]

#define max(a, b) ((a) > (b) ? (a) : (b))

const int kNum = 256;
const int kKernel = 5;
const int kImSize = 224;
const int kInImSize = 228;
const int kOutImSize = 112;

void CnnKernel(
  tapa::mmap<float> input,
  tapa::mmap<float> weight,
  tapa::mmap<float> bias,
  tapa::mmap<float> output);

// Sequential CNN implementation
void CnnSequential(
    aligned_vector<float> & input,
    aligned_vector<float> & weight, 
    aligned_vector<float> & bias,
    aligned_vector<float> & output);

void LoadData(const string& data_dir, 
               aligned_vector<float> & input,
               aligned_vector<float> & weight, 
               aligned_vector<float> & bias);

int Verify(const string& data_dir,
           aligned_vector<float> & output);

#endif
