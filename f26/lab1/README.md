# Lab 1: Dot Product

First, [install the bundled course version of TAPA](../tapa/README.md). The
Makefile selects Xilinx Vivado/Vitis 2024.2 from the NRP Coder workspace.

The kernel computes the dot product of one `float` vector (length 8192) with
itself and outputs the sum. The vector is read as `hls::vector<float, 16>`,
16 floats per access. Four tasks are connected by streams:

```text
input_v mmap --> ReadFromMmap --> DotProduct --> WriteToMmap --> output_sum mmap
                                      |
                                      +--> Timer --> cycle_count mmap
```

`Timer` counts clock cycles until `DotProduct` finishes. The starter
`DotProduct` performs all multiply-adds sequentially.

Run the software simulation:

```bash
cd f26/lab1
make swsim
```

Build the TAPA `.xo` and run fast RTL cosimulation:

```bash
make cosim
```

`make hls` builds only `dot_product.xo`. `make hwemu` remains an alias for
`make cosim`.
