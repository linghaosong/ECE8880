# Lab 0: Vector Addition

First, [install the bundled course version of TAPA](../tapa/README.md). The
Makefile selects Xilinx Vivado/Vitis 2024.2 from the NRP Coder workspace.

The kernel is a four-task dataflow graph connected by `float` streams:

```text
input_v1 mmap --> ReadFromMmap --\
                                  Vadd --> WriteToMmap --> output_v mmap
input_v2 mmap --> ReadFromMmap --/
```

Run the software simulation:

```bash
cd f26/lab0
make swsim
```

Build the TAPA `.xo` and run fast RTL cosimulation:

```bash
make cosim
```

`make hls` builds only `vadd.xo`. `make hwemu` remains an alias for
`make cosim`.
