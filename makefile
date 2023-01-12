INC= ./include
SRC= ./src
BIN= ./bin

sources= main.c bmp.c

all:clean ${BIN}/main ${BIN}/hexviewer

${BIN}/main.o:${SRC}/main.c
	gcc -I ${INC} -c $< -o $@ 

${BIN}/bmp.o:${SRC}/bmp.c
	gcc -I ${INC} -c $< -o $@

${BIN}/main:${BIN}/main.o ${BIN}/bmp.o
	gcc -std=c99 $^ -o $@	

${BIN}/hexviewer:${BIN}/hexviewer.o
	g++  $^ -o $@	

${BIN}/hexviewer.o:${SRC}/hexviewer.cpp
	g++ -c $< -o $@	

clean: 
	rm -r ${BIN} && mkdir ${BIN} 

.PHONY: clean