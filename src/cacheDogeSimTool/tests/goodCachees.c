#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#define MAXVAL 40000

struct wonk{
  int a;
} *shrdPtr;

struct wonk2{
  int a;
} *shrdPtr2;

pthread_mutex_t lock;
pthread_mutex_t lock2;

void *accessorThread(void *arg){

  u_int64_t *result = (u_int64_t*)malloc(sizeof(u_int64_t));
  *result = 0;

  while(*result < MAXVAL){
    pthread_mutex_lock(&lock);
    if(shrdPtr != NULL){
      *result += shrdPtr->a;      
    }
    pthread_mutex_unlock(&lock);
    usleep(1 + (rand() % 2) );
  }

  pthread_exit(result); 
}

void *accessorThread2(void *arg){

  u_int64_t *result2 = (u_int64_t*)malloc(sizeof(u_int64_t));
  *result2 = 0;

  while(*result2 < MAXVAL){
    pthread_mutex_lock(&lock2);//400a4e
    if(shrdPtr2 != NULL){
      *result2 += shrdPtr2->a;      
    }
    pthread_mutex_unlock(&lock2);
    usleep(1 + (rand() % 2) );
  }

  pthread_exit(result2); 
}

int main(int argc, char *argv[]){

  int res = 0;
  int res2 = 0;

  shrdPtr = (struct wonk*)malloc(sizeof(struct wonk));
  shrdPtr->a = 1;
  
  shrdPtr2 = (struct wonk2*)malloc(sizeof(struct wonk2));
  shrdPtr2->a = 2;

  pthread_mutex_init(&lock,NULL);
  pthread_mutex_init(&lock2,NULL);

  pthread_t acc[4];                                               
                                                                //Main is also running on Core 0    
  pthread_create(&acc[0],NULL, accessorThread2,(void*)shrdPtr2);  // Core 1 Active
  usleep(10);
  pthread_create(&acc[1],NULL, accessorThread,(void*)shrdPtr);  // Core 2 Active
  usleep(10);
  pthread_create(&acc[2],NULL, accessorThread,(void*)shrdPtr);  // Core 3 Active
  usleep(10);
  pthread_create(&acc[3],NULL, accessorThread2,(void*)shrdPtr2);  // Core 0 Active
  
  pthread_join(acc[0],(void*)&res);
  pthread_join(acc[1],(void*)&res2);
  pthread_join(acc[2],(void*)&res2);
  pthread_join(acc[3],(void*)&res);
  
  fprintf(stderr,"Final value of res was %d\n",res); 
}