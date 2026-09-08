XILINX_VERSION := 2024.2
XILINX_ROOT := /tools/Xilinx
export XILINX_VITIS := $(XILINX_ROOT)/Vitis/$(XILINX_VERSION)
export XILINX_HLS := $(XILINX_ROOT)/Vitis_HLS/$(XILINX_VERSION)
export XILINX_VIVADO := $(XILINX_ROOT)/Vivado/$(XILINX_VERSION)
export PLATFORM_REPO_PATHS := /opt/xilinx/platforms
export PATH := $(HOME)/.rapidstream-tapa/usr/bin:$(XILINX_VITIS)/bin:$(XILINX_HLS)/bin:$(XILINX_VIVADO)/bin:$(PATH)

GXX_FLAGS := -w -Wno-psabi -O2 -std=c++17 -I$(XILINX_VITIS)/include
# Uncomment to let Vitis HLS reorder floating-point operations in the kernel
# (needed, e.g., for a tree reduction of the float sum). Results may differ
# slightly from the host.
# HLS_CONFIGS := --other-hls-configs "config_compile -unsafe_math_optimizations"
LIB := -ltapa -lfrt -lglog -lgflags -lOpenCL
SRC := ./src

.DEFAULT_GOAL := dot_product
.PHONY: swsim hls cosim hwemu clean cleanall

dot_product.o: $(SRC)/dot_product.cpp $(SRC)/dot_product.h
	tapa g++ -- $(GXX_FLAGS) -c $<

main.o: $(SRC)/main.cpp $(SRC)/dot_product.h
	tapa g++ -- $(GXX_FLAGS) -c $<

dot_product: dot_product.o main.o
	tapa g++ -- $(GXX_FLAGS) -o $@ $^ $(LIB)

swsim: dot_product
	./dot_product

dot_product.xo: $(SRC)/dot_product.cpp $(SRC)/dot_product.h
	tapa compile --top DotProductKernel \
	--platform xilinx_u55c_gen3x16_xdma_3_202210_1 \
	--clock-period 5.00 \
	$(HLS_CONFIGS) \
	-f $< \
	-o $@

hls: dot_product.xo

cosim: dot_product dot_product.xo
	env -u XILINX_XRT ./dot_product --btstm=./dot_product.xo

hwemu: cosim

clean:
	rm -f *.o dot_product

cleanall: clean
	rm -f dot_product.xo
	rm -rf work.out
