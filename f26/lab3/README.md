# Lab 3 Part 1: Convolutional Neural Network (CNN) Dataflow

First, [install the bundled course version of TAPA](../tapa/README.md). The
Makefile selects Xilinx Vivado/Vitis 2024.2 from the NRP Coder workspace.

This lab is about a one-layer convolutional neural network. The layer applies
a `kKernel` x `kKernel` convolution with bias for every input/output channel
pair, then ReLU, then 2 x 2 max pooling:

```text
in_img [kNum][kInImSize][kInImSize]   (input image, one channel per input)
weight [kNum][kNum][kKernel][kKernel] (output channel, input channel, p, q)
bias   [kNum]
out_img[kNum][kOutImSize][kOutImSize] (after conv + bias + ReLU + maxpool)
```

with compile-time parameters from [src/cnn.h](src/cnn.h):

```cpp
constexpr int kNum = 256;       // channel number
constexpr int kKernel = 5;      // kernel size
constexpr int kImSize = 224;    // image size (after conv)
constexpr int kInImSize = kImSize + kKernel - 1;  // input image size
constexpr int kOutImSize = kImSize / 2; // output image size (after maxpool)
```

Because the parameters are compile-time constants, HLS knows every loop trip
count; you do not need `tripcount` pragmas.

CNN weights and loading functions are reused from UCLA CS 259 21F Lab 2.

## Dataset

This lab includes prepared data in [data/](data); no separate download is
required. Each file is a flat little-endian `float` array in the layout above.

| File in `data/` | Contents | Size |
| --- | --- | --- |
| `input.bin` | Input image, `kNum * kInImSize * kInImSize` floats | 53,231,616 bytes |
| `weight.bin` | Weights, `kNum * kNum * kKernel * kKernel` floats | 6,553,600 bytes |
| `bias.bin` | Biases, `kNum` floats | 1,024 bytes |
| `output.bin` | Reference output, `kNum * kOutImSize * kOutImSize` floats | 12,845,056 bytes |

## Kernel data flow

The kernel is a TAPA dataflow design. The starter declares the five task
signatures with empty bodies; you write the whole dataflow program: the task
bodies, the stream declarations, and the task connections in `CnnKernel`.
`read_input`, `read_weight`, and `read_bias` move data from memory (mmap) to
streams; `cnncore` consumes the three input streams, computes the layer, and
produces the output stream; `write_output` moves the output stream back to
memory:

```text
in_img mmap --> read_input --in_img_stream--+
                                             |
weight mmap --> read_weight --weight_stream--+--> cnncore --out_img_stream--> write_output --> out_img mmap
                                             |
bias mmap   --> read_bias ----bias_stream----+
```

The diagram shows one `cnncore` instance. For parallel processing, deploy
multiple `cnncore` instances and split the output channels across them —
instance *c* computes output channels `c * kNum / N` to
`(c + 1) * kNum / N - 1` of `N` instances. A stream read consumes a value, so
the tasks cannot share one input stream; declare one set of streams per
instance and adjust the supplied task signatures and their invokes in
`CnnKernel` accordingly:

- `read_input` broadcasts the full input image to every instance (one
  `in_img_stream` per instance, as in Lab 2's `ReadTestImages`).
- `read_weight` and `read_bias` send each instance only the weights and
  biases of its own output channels (one stream per instance).
- `write_output` gathers one output stream per instance and writes the
  channels to `out_img` in order.

A stream read consumes a value, so every producer/consumer pair must agree on
the number of values. For one kernel invocation with a single `cnncore`
instance, the required counts are:

| Channel | Values per stream |
| --- | --- |
| `in_img_stream` | `kNum * kInImSize * kInImSize` floats |
| `weight_stream` | `kNum * kNum * kKernel * kKernel` floats |
| `bias_stream` | `kNum` floats |
| `out_img_stream` | `kNum * kOutImSize * kOutImSize` floats |

With `N` instances, each instance's `in_img_stream` still carries the full
input image, while its weight, bias, and output streams carry only its
`kNum / N` output channels' share.

Declare all streams in `CnnKernel` with depth `kStreamDepth = 2` (defined in
[src/cnn.h](src/cnn.h)). Use `.read()` and `.write()` for stream access.

## Run and verify

Complete every `...` placeholder in [src/cnn.cpp](src/cnn.cpp), following the
comments above each task: implement the five task bodies, declare the
streams, and connect all tasks in `CnnKernel` (be careful about the argument
order when connecting streams). The starter code will not compile until all
placeholders are replaced. Keep the CPU reference and verification code in
[src/main.cpp](src/main.cpp) unchanged.

Then, from the repository root, run the software simulation:

```bash
cd f26/lab3
make swsim
```

Run commands from `f26/lab3`; the default data path is `./data` (override with
`./cnn --dtf=/path/to/data`). [src/main.cpp](src/main.cpp) loads the data,
runs the CPU reference (`CnnSequential`), invokes the kernel, and verifies the
kernel output against the CPU output. `PASS` means the outputs match; a
mismatch prints `FAIL` and returns a nonzero exit status. The Makefile sets
`TAPA_CONCURRENCY=8` for the software-simulation task workers; set it in the
environment as well when running `./cnn` directly.

When debugging, it is better to test on a smaller CNN. Since the parameters
are compile-time constants, comment out the default parameters in
[src/cnn.h](src/cnn.h) and uncomment the small setting
(`kNum = 32`, `kKernel = 3`, `kImSize = 50`), then rebuild with
`make clean && make swsim`. The host only compares against its own CPU
reference, so the small setting works with the same data files. Always set the
parameters back to the default values before the final check and before
running HLS.

Part 1 is complete when `make swsim` prints `PASS` with the default
parameters. The `hls` and `cosim` targets are used in Part 2.

After a kernel change, rerun `make swsim` and check for `PASS`. The
software-simulation time printed by the host is not an FPGA performance
measurement.
