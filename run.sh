#!/bin/bash

# Compile
mpicc -Wall -O3 -o out main.c reader/fileReader.c sort/bitonicSort.c -lm

# Run
for (( i = 0; i < 5; i++ )); do
  mpiexec -n 8 ./out files/datSeq16M.bin
done