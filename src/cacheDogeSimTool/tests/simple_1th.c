/*
 * Created on Mon Nov 19 2018
 * Author: Artur Balanuta & Nidhi Bhatia
 * Copyright (c) 2018 Carnegie Mellon University
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

#define SIZE 100
int main(int argc, char *argv[]){


  int *array = (int*) memalign(64, SIZE * sizeof(int));
  int sum = 0;
  
  for (int i = 0; i < SIZE; i++){
    for (int j = 0; j < SIZE; j++){
      array[j] = j*i; 
    }
  }
  

  // for (int i = 0; i < SIZE; i++){
  //   sum += array[i]; 
  // }

  printf("Final value of sum was %d \n", sum ); 
}