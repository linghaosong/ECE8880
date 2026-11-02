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
DATA := ../../lab3/data

# HLS clock period (ns); implementation targets the same kernel frequency.
CLOCK_PERIOD := 3.33
KERNEL_FREQ := $(shell awk 'BEGIN { printf "%.0f", 1000 / $(CLOCK_PERIOD) }')
PLATFORM := xilinx_u55c_gen3x16_xdma_3_202210_1
SCRIPTS := ../scripts
IMPL_RUN := _x/link/vivado/vpl/prj/prj.runs/impl_1
# Final implementation checkpoint, after post-route physical optimization.
FINAL_DCP := $(IMPL_RUN)/level0_wrapper_postroute_physopt.dcp
KERNEL_CELL := level0_i/ulp/CnnKernel_1
# A stub libudev keeps Vivado's license check from crashing on workspaces
# without udev; it is loaded for the Vivado tools only.
VIVADO_ENV := env LD_LIBRARY_PATH=$(CURDIR)/compat:$(XILINX_VIVADO)/lib/lnx64.o:$$LD_LIBRARY_PATH

.DEFAULT_GOAL := cnn
.PHONY: swsim hls impl util layout clean cleanall

cnn.o: $(SRC)/cnn.cpp $(SRC)/cnn.h Makefile
	tapa g++ -- $(GXX_FLAGS) -c $<

main.o: $(SRC)/main.cpp $(SRC)/cnn.h Makefile
	tapa g++ -- $(GXX_FLAGS) -c $<

cnn: cnn.o main.o
	tapa g++ -- $(GXX_FLAGS) -o $@ $^ $(LIB)

swsim: cnn
	./cnn --dtf=$(DATA)

cnn.xo: $(SRC)/cnn.cpp $(SRC)/cnn.h Makefile
	tapa compile -j 8 --top CnnKernel \
	--platform $(PLATFORM) \
	--clock-period $(CLOCK_PERIOD) \
	-f $< \
	-o $@

hls: cnn.xo

compat/libudev.so.1: $(SCRIPTS)/udev_stub.c
	mkdir -p compat
	gcc -shared -fPIC -o $@ $<

# Vivado synthesis, placement, and routing in the U55C platform.
# link.cfg assigns each kernel port to an HBM bank.
cnn.xclbin: cnn.xo link.cfg compat/libudev.so.1 $(SCRIPTS)/clkwiz_compat.tcl
	$(VIVADO_ENV) v++ --link --target hw --platform $(PLATFORM) \
	--config link.cfg \
	--kernel_frequency $(KERNEL_FREQ) \
	--vivado.prop run.impl_1.STEPS.INIT_DESIGN.TCL.PRE=$(abspath $(SCRIPTS)/clkwiz_compat.tcl) \
	--vivado.prop run.impl_1.STEPS.POST_ROUTE_PHYS_OPT_DESIGN.IS_ENABLED=true \
	--save-temps \
	-o $@ $<

impl: cnn.xclbin

# Routed utilization of the kernel and each task, to compare with HLS.
kernel_util.rpt: $(FINAL_DCP) compat/libudev.so.1
	$(VIVADO_ENV) vivado -mode batch -nojournal -nolog -notrace \
	-source $(SCRIPTS)/kernel_util.tcl -tclargs $(FINAL_DCP) $(KERNEL_CELL) $@

util: kernel_util.rpt

# Open the final checkpoint in the Vivado GUI (run from the workspace desktop).
layout: compat/libudev.so.1
	$(VIVADO_ENV) vivado -mode gui -nojournal -nolog \
	-source $(SCRIPTS)/layout.tcl -tclargs $(FINAL_DCP) $(KERNEL_CELL)

clean:
	rm -f *.o cnn

cleanall: clean
	rm -f cnn.xo cnn.xclbin* *.log *.jou *.rpt
	rm -rf work.out _x compat .Xil .ipcache
