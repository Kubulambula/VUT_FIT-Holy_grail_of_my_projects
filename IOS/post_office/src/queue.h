// Janšta Jakub (xjanst02)


#ifndef QUEUE
#define QUEUE
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>

typedef struct queue_t{
    unsigned size;
    unsigned element_count;
    sem_t** queue;
} queue_t;

#include "util.h"

queue_t* queue_init(unsigned size);

void queue_free(queue_t* self);

int queue_append(queue_t* self, sem_t* sem);

sem_t* queue_pop(queue_t* self);

void queue_print(queue_t* self);
#endif