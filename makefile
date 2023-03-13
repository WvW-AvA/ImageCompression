INC= ./include
SRC= ./src
BIN= ./bin
OPTION= -g

all:clean ${BIN}/main ${BIN}/hexviewer

${BIN}/main:${BIN}/main.o ${BIN}/bmp.o ${BIN}/huffman.o ${BIN}/prediction.o ${BIN}/lz77.o ${BIN}/golomb.o ${BIN}/jls.o
	gcc -std=c99 $^ -o $@ ${OPTION} -lm

${BIN}/hexviewer:${BIN}/hexviewer.o
	g++  $^ -o $@ ${OPTION}	

${BIN}/main.o:${SRC}/main.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION} 

${BIN}/golomb.o:${SRC}/golomb.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION} -lm

${BIN}/jls.o:${SRC}/jls.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION} -lm

${BIN}/bmp.o:${SRC}/bmp.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION}

${BIN}/huffman.o:${SRC}/huffman.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION}

${BIN}/prediction.o:${SRC}/prediction.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION}

${BIN}/lz77.o:${SRC}/lz77.c
	gcc -std=c99 -I ${INC} -c $< -o $@ ${OPTION}


${BIN}/hexviewer.o:${SRC}/hexviewer.cpp
	g++ -c $< -o $@ ${OPTION}	

clean: 
	rm -r ${BIN} && mkdir ${BIN} 

.PHONY: clean