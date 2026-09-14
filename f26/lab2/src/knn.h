#ifndef KNN_H_
#define KNN_H_

#include <cstdint>
#include <tapa.h>

// Student starter: every `...` in main.cpp, knn.h, and knn.cpp marks missing
// code (a type, arguments, or statements). Replace all placeholders before
// building; they are not valid C++ here. See the TODO comments for requirements.

// Shared image layout for the host and kernel: 3072 bytes, or 768 packed words.
// Each byte is one color-channel value; each uint32_t carries four bytes.
constexpr int kNumClasses = 10;
constexpr int kImageBytes = 32 * 32 * 3;
constexpr int kBytesPerWord = sizeof(uint32_t);
constexpr int kImageWords = kImageBytes / kBytesPerWord;
constexpr int kStreamDepth = 2;

// The Makefile supplies the same -D definitions to the host and HLS compiler.
// Use these constants directly in task loops so HLS knows their trip counts.
// Training images are counted per class; test images are counted in total.
constexpr int kTrainImagesPerClass = TRAIN_IMAGE_NUM;
constexpr int kTestImages = TEST_IMAGE_NUM;
static_assert(kTrainImagesPerClass > 0 && kTrainImagesPerClass <= 5000,
              "TRAIN_IMAGE_NUM must be between 1 and 5000");
static_assert(kTestImages > 0 && kTestImages <= 10000,
              "TEST_IMAGE_NUM must be between 1 and 10000");

// Training mmap c contains only class c. The kernel writes one label per test
// image, using kTrainImagesPerClass training images from each class.
// TODO: Complete the image-port element types. Keep this declaration, the
// definition in knn.cpp, and the host's mmap views in main.cpp consistent.
void KNNKernel(
    tapa::mmap<...> train_image_0,
    tapa::mmap<...> train_image_1,
    tapa::mmap<...> train_image_2,
    tapa::mmap<...> train_image_3,
    tapa::mmap<...> train_image_4,
    tapa::mmap<...> train_image_5,
    tapa::mmap<...> train_image_6,
    tapa::mmap<...> train_image_7,
    tapa::mmap<...> train_image_8,
    tapa::mmap<...> train_image_9,
    tapa::mmap<...> test_image,
    tapa::mmap<uint32_t> predict_label,
    tapa::mmap<uint32_t> cycle_count);

#endif
