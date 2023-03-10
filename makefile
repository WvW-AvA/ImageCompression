INC= ./include
SRC= ./src
BIN= ./bin

all:clean ${BIN}/main ${BIN}/hexviewer

${BIN}/main:${BIN}/main.o ${BIN}/bmp.o ${BIN}/huffman.o ${BIN}/prediction.o ${BIN}/lz77.o ${BIN}/golomb.o
	gcc -std=c99 $^ -o $@ -lm

${BIN}/hexviewer:${BIN}/hexviewer.o
	g++  $^ -o $@	

${BIN}/main.o:${SRC}/main.c
	gcc -std=c99 -I ${INC} -c $< -o $@ 

${BIN}/golomb.o:${SRC}/golomb.c
	gcc -std=c99 -I ${INC} -c $< -o $@ -lm

${BIN}/bmp.o:${SRC}/bmp.c
	gcc -std=c99 -I ${INC} -c $< -o $@

${BIN}/huffman.o:${SRC}/huffman.c
	gcc -std=c99 -I ${INC} -c $< -o $@

${BIN}/prediction.o:${SRC}/prediction.c
	gcc -std=c99 -I ${INC} -c $< -o $@

${BIN}/lz77.o:${SRC}/lz77.c
	gcc -std=c99 -I ${INC} -c $< -o $@


${BIN}/hexviewer.o:${SRC}/hexviewer.cpp
	g++ -c $< -o $@	

clean: 
	rm -r ${BIN} && mkdir ${BIN} 

.PHONY: clean