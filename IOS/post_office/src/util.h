// Janšta Jakub (xjanst02)


#ifndef UTIL
#define UTIL
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "queue.h"


int get_arg_number(char* arg);

int get_args(int argc, char* argv[], int* NZ, int* NU, int* TZ, int* TU, int* F);

void randomize();

int randi(int min, int max);

void shuffle(int* array, size_t n);

sem_t* create_shared_semaphore(int init_val);

void free_shared_semaphore(sem_t* sem);

queue_t* create_shared_queue(unsigned size);

void free_shared_queue(queue_t* queue);

#endif