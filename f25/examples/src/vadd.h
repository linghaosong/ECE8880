#ifndef VADD_H_
#define VADD_H_

#include <string>
#include <tapa.h>

using std::string;

void VaddKernel(
    tapa::mmap<float> input_v1,
    tapa::mmap<float> input_v2,
    tapa::mmap<float> output_v,
    const int vlen);

#endif
