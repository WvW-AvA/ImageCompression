INC= ./include
SRC= ./src
BIN= ./bin

all:clean ${BIN}/main ${BIN}/hexviewer

${BIN}/main:${BIN}/main.o ${BIN}/bmp.o ${BIN}/huffman.o ${BIN}/prediction.o
	gcc -std=c99 $^ -o $@	

${BIN}/hexviewer:${BIN}/hexviewer.o
	g++  $^ -o $@	

${BIN}/main.o:${SRC}/main.c
	gcc -I ${INC} -c $< -o $@ 

${BIN}/bmp.o:${SRC}/bmp.c
	gcc -I ${INC} -c $< -o $@

${BIN}/huffman.o:${SRC}/huffman.c
	gcc -I ${INC} -c $< -o $@

${BIN}/prediction.o:${SRC}/prediction.c
	gcc -I ${INC} -c $< -o $@

${BIN}/hexviewer.o:${SRC}/hexviewer.cpp
	g++ -c $< -o $@	

clean: 
	rm -r ${BIN} && mkdir ${BIN} 

.PHONY: clean