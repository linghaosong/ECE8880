XILINX_VERSION := 2024.2
XILINX_ROOT := /tools/Xilinx
export XILINX_VITIS := $(XILINX_ROOT)/Vitis/$(XILINX_VERSION)
export XILINX_HLS := $(XILINX_ROOT)/Vitis_HLS/$(XILINX_VERSION)
export XILINX_VIVADO := $(XILINX_ROOT)/Vivado/$(XILINX_VERSION)
export PLATFORM_REPO_PATHS := /opt/xilinx/platforms
export PATH := $(HOME)/.rapidstream-tapa/usr/bin:$(XILINX_VITIS)/bin:$(XILINX_HLS)/bin:$(XILINX_VIVADO)/bin:$(PATH)

GXX_FLAGS := -w -O2 -std=c++17 -I$(XILINX_VITIS)/include
# Use eight TAPA task workers during software simulation.
export TAPA_CONCURRENCY := 8
LIB := -ltapa -lfrt -lglog -lgflags -lOpenCL
SRC := ./src

.DEFAULT_GOAL := cnn
.PHONY: swsim hls cosim hwemu clean cleanall

cnn.o: $(SRC)/cnn.cpp $(SRC)/cnn.h Makefile
	tapa g++ -- $(GXX_FLAGS) -c $<

main.o: $(SRC)/main.cpp $(SRC)/cnn.h Makefile
	tapa g++ -- $(GXX_FLAGS) -c $<

cnn: cnn.o main.o
	tapa g++ -- $(GXX_FLAGS) -o $@ $^ $(LIB)

swsim: cnn
	./cnn

cnn.xo: $(SRC)/cnn.cpp $(SRC)/cnn.h Makefile
	tapa compile -j 8 --top CnnKernel \
	--platform xilinx_u55c_gen3x16_xdma_3_202210_1 \
	--clock-period 3.33 \
	-f $< \
	-o $@

hls: cnn.xo

cosim: cnn cnn.xo
	env -u XILINX_XRT ./cnn --btstm=./cnn.xo

hwemu: cosim

clean:
	rm -f *.o cnn

cleanall: clean
	rm -f cnn.xo
	rm -rf work.out
