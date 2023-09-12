// Janšta Jakub (xjanst02)


#include "util.h"


int get_arg_number(char* arg){
    for (int i = 0; i < strlen(arg); i++){
        if (!isdigit(arg[i])){
            return -1;
        }
    }
    return atoi(arg);
}


// loads and validates args. If args are not valid, handles error output and returns 1. Returns 0 on success
int get_args(int argc, char* argv[], int* NZ, int* NU, int* TZ, int* TU, int* F){
    if (argc != 6) {
        fprintf(stderr, "Argument count mismatch\nUsage: NZ NU TZ TU F\n");
        return 1;
    }

    *NZ = get_arg_number(argv[1]);
    *NU = get_arg_number(argv[2]);
    *TZ = get_arg_number(argv[3]);
    *TU = get_arg_number(argv[4]);
    *F = get_arg_number(argv[5]);

    int err = 0;
    // this is a syntax herecy, but at least it is readable
    if (*NZ < 0) { fprintf(stderr, "Argument NZ is out of allowed range (0 < NZ )\n"); err = 1; }
    if (*NU <= 0) { fprintf(stderr, "Argument NU is out of allowed range (0 <= NU )\n"); err = 1; }
    if (*TZ < 0 || *TZ > 10000) { fprintf(stderr, "Argument TZ is out of allowed range (0 <= TZ <= 10000 )\n"); err = 1; }
    if (*TU < 0 || *TU > 100) { fprintf(stderr, "Argument TU is out of allowed range (0 <= TU <= 100 )\n"); err = 1; }
    if (*F < 0 || *F > 10000) { fprintf(stderr, "Argument F is out of allowed range (0 <= F <= 1000)\n"); err = 1; }

    return err;
}


void randomize(){
    srand(time(0));
}


int randi(int min, int max){
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}


void shuffle(int* array, size_t n){
    if (n > 1){
        size_t i;
        for (i = 0; i < n - 1; i++){
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}


sem_t* create_shared_semaphore(int init_val){
    sem_t* sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (sem == MAP_FAILED){
        return NULL;
    }
    if(sem_init(sem, 1, init_val)){ // error inicializing the semaphore
        munmap(sem, sizeof(sem_t*));
        return NULL;
    }
    return sem;
}


void free_shared_semaphore(sem_t* sem){
    sem_close(sem);
    munmap(sem, sizeof(sem_t));
}


queue_t* create_shared_queue(unsigned size){
    /*queue_t* q = queue_init(size);
    if (q == NULL){
        return NULL;
    }
    return q;*/
    return queue_init(size);
}


void free_shared_queue(queue_t* queue){
    queue_free(queue);
    munmap(queue, sizeof(queue_t));
}

