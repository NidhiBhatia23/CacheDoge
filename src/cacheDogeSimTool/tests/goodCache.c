#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#define MAXVAL 40000

struct wonk{
  int a;
} *shrdPtr;

pthread_mutex_t lock;

struct wonk *getNewVal(struct wonk**old){
  free(*old);
  *old = NULL;
  struct wonk *newval = (struct wonk*)malloc(sizeof(struct wonk));
  newval->a = 1;
  return newval;
}

void *updaterThread(void *arg){

  int i;
  for(i = 0; i < 10; i++){    
    pthread_mutex_lock(&lock);
    struct wonk *newval = getNewVal(&shrdPtr);
    shrdPtr = newval;
    pthread_mutex_unlock(&lock);
    usleep(10 + (rand() % 100) );
  }

}

void *sleeperThread(void *arg){
  usleep(2000);
}

void *accessorThread(void *arg){

  u_int64_t *result = (u_int64_t*)malloc(sizeof(u_int64_t));; 
  *result = 0;

  while(*result < MAXVAL){
    pthread_mutex_lock(&lock);//400a4e
    if(shrdPtr != NULL){
      *result += shrdPtr->a;      
    }
    pthread_mutex_unlock(&lock);
    usleep(1 + (rand() % 2) );
  }

  pthread_exit(result); 
}

int main(int argc, char *argv[]){

  int res = 0;
  shrdPtr = (struct wonk*)malloc(sizeof(struct wonk));
  shrdPtr->a = 1;

  pthread_mutex_init(&lock,NULL);

  pthread_t acc[4];                                               
                                                                //Main is also running on Core 0    
  pthread_create(&acc[0],NULL, sleeperThread,(void*)shrdPtr);  // Core 1 Sleeping
  usleep(10);
  pthread_create(&acc[1],NULL, accessorThread,(void*)shrdPtr);  // Core 2 Active
  usleep(10);
  pthread_create(&acc[2],NULL, accessorThread,(void*)shrdPtr);  // Core 3 Active

  pthread_join(acc[0],(void*)&res);
  pthread_join(acc[1],(void*)&res);
  pthread_join(acc[2],(void*)&res);
  
  fprintf(stderr,"Final value of res was %d\n",res); 
}