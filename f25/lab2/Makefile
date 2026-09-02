INC_XCL := 
#-I /opt/xilinx/xrt/include/
GXX_FLAGS := -w -O2 -std=c++17
LIB := -ltapa -lfrt -lglog -lgflags -lOpenCL
SRC := ./src

Platform := xilinx_u55c_gen3x16_xdma_3_202210_1

.DEFAULT_GOAL := knn

knn.o: $(SRC)/knn.cpp
	tapa g++ -- $(GXX_FLAGS) -c $^ $(INC_XCL)

main.o: $(SRC)/main.cpp
	tapa g++ -- $(GXX_FLAGS) -c $^ $(INC_XCL)

knn: knn.o main.o
	tapa g++ -- $(GXX_FLAGS) -o $@ $^ $(INC_XCL) $(LIB)

swsim: knn
	./knn --skipk=false --train_num=8 --test_num=16

hls: $(SRC)/knn.cpp
	tapa compile --top KNNKernel \
	--platform xilinx_u55c_gen3x16_xdma_3_202210_1 \
	--clock-period 4.4 \
	-f $^ \
	-o knn.xo

hwemu: knn.xo
	./knn --skipk=false --btstm=./knn.xo --train_num=4 --test_num=8

clean:
	rm *.o knn

cleanall:
	rm *.o knn knn.xo
	rm -rf work.out
