// Janšta Jakub (xjanst02)


# include "queue.h"


// Creates a FIFO queue for semaphores
queue_t* queue_init(unsigned size){
    // allocate space for the struct
    queue_t* q = mmap(NULL, sizeof(queue_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (q == MAP_FAILED){
        return NULL;
    }
    // allocate space for the semaphore array
    q->queue = mmap(NULL, sizeof(sem_t*) * size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (q->queue == MAP_FAILED){
        return NULL;
    }

    #ifdef DEBUG
    memset(q->queue, 0, sizeof(sem_t*) * size); // set all to NULL so that debug print is more readable
    #endif

    q->size = size;
    q->element_count = 0;
    return q;
}


void queue_free(queue_t* self){
    // free the semaphores in the array
    for(int i = 0; i < self->element_count; i++){
        free_shared_semaphore(self->queue[i]);
    }
    // free the array
    munmap(self->queue, sizeof(sem_t*) * self->size);
    // free the struct
    munmap(self, sizeof(queue_t));
}


int queue_append(queue_t* self, sem_t* sem){
    if(self->element_count < self->size){
        self->queue[self->element_count++] = sem;
        return 0;
    }
    #ifdef DEBUG
    fprintf(stderr, "queue_append(): Attempted to append behind queue end. Ignored...\n");
    #endif
    return 1;
}


sem_t* queue_pop(queue_t* self){
    if (self->element_count == 0){ // queue is empty so return NULL
        return NULL;
    }
    
    sem_t* out = self->queue[0];
    // reorder the queue
    for (int i = 0; i < self->element_count; i++){
        self->queue[i] = self->queue[i+1];
    }
    self->element_count--;
    return out;
}


void queue_print(queue_t* self){
    // Fancy debug print
    #ifdef DEBUG
    printf("=== Queue: size = %d, element_count = %d, queue[] = %p ===\n[", self->size, self->element_count, self->queue);
    for (int i = 0; i < self->size; i++){
        printf("%p", self->queue[i]);
        if (i < self->size-1){
            printf(", ");
        }
    }
    printf("]\n");
    #endif
}