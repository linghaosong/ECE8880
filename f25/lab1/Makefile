INC_XCL := 
#-I /opt/xilinx/xrt/include/
GXX_FLAGS := -w -O2 -std=c++17
LIB := -ltapa -lfrt -lglog -lgflags -lOpenCL
SRC := ./src

.DEFAULT_GOAL := vadd

vadd.o: $(SRC)/vadd.cpp
	tapa g++ -- $(GXX_FLAGS) -c $^ $(INC_XCL)

main.o: $(SRC)/main.cpp
	tapa g++ -- $(GXX_FLAGS) -c $^ $(INC_XCL)

vadd: vadd.o main.o
	tapa g++ -- $(GXX_FLAGS) -o $@ $^ $(INC_XCL) $(LIB)

swsim: vadd
	./vadd

hls: $(SRC)/vadd.cpp
	tapa compile --top VaddKernel \
	--platform xilinx_u55c_gen3x16_xdma_3_202210_1 \
	--clock-period 5.00 \
	-f $^ \
	-o vadd.xo

hwemu: vadd.xo
	./vadd --btstm=./vadd.xo

clean:
	rm *.o vadd

cleanall:
	rm *.o vadd vadd.xo
	rm -rf work.out