XILINX_VERSION := 2024.2
XILINX_ROOT := /tools/Xilinx
export XILINX_VITIS := $(XILINX_ROOT)/Vitis/$(XILINX_VERSION)
export XILINX_HLS := $(XILINX_ROOT)/Vitis_HLS/$(XILINX_VERSION)
export XILINX_VIVADO := $(XILINX_ROOT)/Vivado/$(XILINX_VERSION)
export PLATFORM_REPO_PATHS := /opt/xilinx/platforms
export PATH := $(HOME)/.rapidstream-tapa/usr/bin:$(XILINX_VITIS)/bin:$(XILINX_HLS)/bin:$(XILINX_VIVADO)/bin:$(PATH)

GXX_FLAGS := -w -O2 -std=c++17
LIB := -ltapa -lfrt -lglog -lgflags -lOpenCL
SRC := ./src

.DEFAULT_GOAL := vadd
.PHONY: swsim hls cosim hwemu clean cleanall

vadd.o: $(SRC)/vadd.cpp $(SRC)/vadd.h
	tapa g++ -- $(GXX_FLAGS) -c $<

main.o: $(SRC)/main.cpp $(SRC)/vadd.h
	tapa g++ -- $(GXX_FLAGS) -c $<

vadd: vadd.o main.o
	tapa g++ -- $(GXX_FLAGS) -o $@ $^ $(LIB)

swsim: vadd
	./vadd

vadd.xo: $(SRC)/vadd.cpp $(SRC)/vadd.h
	tapa compile --top VaddKernel \
	--platform xilinx_u55c_gen3x16_xdma_3_202210_1 \
	--clock-period 5.00 \
	-f $< \
	-o $@

hls: vadd.xo

cosim: vadd vadd.xo
	env -u XILINX_XRT ./vadd --btstm=./vadd.xo

hwemu: cosim

clean:
	rm -f *.o vadd

cleanall: clean
	rm -f vadd.xo
	rm -rf work.out
