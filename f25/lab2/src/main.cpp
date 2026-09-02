#include <chrono>
#include <iostream>
#include <string>
#include <cmath>

#include "knn.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;
using std::clog;
using std::endl;
using std::string;

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(btstm, "", "path to the bitstream file, run csim if empty");
DEFINE_string(data, "./cifar-10", "path to the CIFA10 binary data folder");
DEFINE_int32(train_num, 32, "number of training images per class");
DEFINE_int32(test_num, 32, "number of test images");
DEFINE_bool(skipk, true, "skip kernel execution, only host CPU if true");

//read binary file and store the content into one vector uint8_t
template <typename T>
void ReadBinaryFile(const string &filename, aligned_vector<T> &data) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Cannot open file: " + filename);
  }
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);
  data.resize(size);
  file.read(reinterpret_cast<char*>(data.data()), size);
  if (!file) {
    throw std::runtime_error("Error reading file: " + filename);
  }
  file.close();
  clog << "Read " << size << " bytes from " << filename << endl; 
}

// KNN on host for result verification and host CPU performance benchmark
// we use 1-NN, K=1
void KNN_host(std::vector<aligned_vector<uint8_t> > & train_image,
              aligned_vector<uint8_t> & test_image,
              aligned_vector<uint32_t> & predict_label,
              const int test_image_num,
              const int train_image_each_class_num = 100) {
  predict_label.resize(test_image_num);
  for (int t = 0; t < test_image_num; t++) {
    int best_label = 0;
    int best_dist = 0x7FFFFFFF;
    for (int tr = 0; tr < train_image_each_class_num; tr++) {
      for (int c = 0; c < 10; ++c) {
        int dist = 0;
        for (int i = 0; i < 3072; ++i) {
          int d = (int)train_image[c][tr * 3072 + i] - (int)test_image[t * 3072 + i];
          dist += d * d;
        }
        if (dist < best_dist) {
          best_dist = dist;
          best_label = c;
        }
      }
    }
    predict_label[t] = best_label;
    if (t % 1000 == 0) {
      clog << "Processed " << t << " test images" << endl;
    }
  }
}

float Verify_predcition_accuracy(
    aligned_vector<uint32_t> & test_label,
    aligned_vector<uint32_t> & predict_label) {
  const int test_image_num = predict_label.size();
  int correct = 0;
  for (int i = 0; i < test_image_num; i++) {
    correct += (test_label[i] == predict_label[i]);
  }
  clog << "Correct: " << correct << " out of " << test_image_num << endl;
  clog << "Accuracy: "
       << (float)correct / (float)test_image_num * 100.0f
       << "%" << endl;
  return (float)correct / (float)test_image_num;
}

bool end_with(const std::string &value, const std::string &ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  //read cifar-10 train images
  std::vector<aligned_vector<uint8_t> > train_image(10);
  for (int i = 0; i < 10; ++i) {
    ReadBinaryFile(FLAGS_data + "/train_image_" + std::to_string(i) + ".bin", train_image[i]);
  }

  //read cifar-10 test images and labels
  aligned_vector<uint8_t> test_image;
  aligned_vector<uint32_t> test_label;
  ReadBinaryFile(FLAGS_data + "/test_image.bin", test_image);
  ReadBinaryFile(FLAGS_data + "/test_label.bin", test_label);

  aligned_vector<uint32_t> predict_label;

  // add a timer to measure host KNN performance
  steady_clock::time_point t1 = steady_clock::now();
  KNN_host(train_image, test_image, predict_label, FLAGS_test_num, FLAGS_train_num);
  steady_clock::time_point t2 = steady_clock::now();
  double time_taken = duration_cast<milliseconds>(t2 - t1).count();
  clog << "Host CPU KNN time: " << time_taken << " millisecond" << endl;

  // veryfy host KNN prediction aginst test label
  clog << "Verifying host KNN (on CPU) prediction accuracy..." << endl;
  float acc_cpu = Verify_predcition_accuracy(test_label, predict_label);

  if (FLAGS_skipk) {
    return EXIT_SUCCESS;
  }

  //KNN Kernel execution on FPGA(or CPU in csim mode and hw emulation mode
  if (FLAGS_btstm.empty()) {
    clog << "Runing KNN kernel in csim mode" << endl;
  } else if (end_with(FLAGS_btstm, ".xo")) {
    clog << "Runing KNN kernel in TAPA fast cosim using file: " << FLAGS_btstm << endl;
  } else if (end_with(FLAGS_btstm, ".hw_emu.xclbin")) {
    clog << "Runing KNN kernel in Vitis hardware emulation using file: " << FLAGS_btstm << endl;
  } else if (end_with(FLAGS_btstm, ".xclbin")) {
    clog << "Runing KNN kernel on FPGA card using bitstream file: " << FLAGS_btstm << endl;
  } else {
    throw std::runtime_error("Unsupported bitstream file: " + FLAGS_btstm);
    return EXIT_FAILURE;
  }

  aligned_vector<uint32_t> cycle_count(1);

  time_taken =
  tapa::invoke(KNNKernel, 
               FLAGS_btstm,
               tapa::read_only_mmap<uint8_t>(train_image[0]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[1]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[2]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[3]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[4]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[5]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[6]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[7]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[8]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(train_image[9]).reinterpret<uint32_t>(),
               tapa::read_only_mmap<uint8_t>(test_image).reinterpret<uint32_t>(),
               tapa::write_only_mmap<uint32_t>(predict_label),
               tapa::write_only_mmap<uint32_t>(cycle_count),
               FLAGS_test_num,
               FLAGS_train_num);
  
  clog << "KNN kernel execution time: " << time_taken * 1e-6 << " millisecond" << endl;
  clog << "KNN kernel cycle count: " << cycle_count[0] << endl;

  // veryfy KNN kernel prediction aginst test label
  clog << "Verifying KNN kernel prediction accuracy..." << endl;
  float acc_kernel = Verify_predcition_accuracy(test_label, predict_label);

  if (std::abs(acc_cpu - acc_kernel) * 100.0f > 0.1f) {
    clog << "Accuracy mismatch: KNN kernel accuracy " << acc_kernel * 100.0f
         << "% differs from host CPU accuracy " << acc_cpu * 100.0f
         << "% by more than 0.1%" << endl;
  } else {
    clog << "KNN kernel test PASS!" << endl;
  }
  
  return EXIT_SUCCESS;
}
