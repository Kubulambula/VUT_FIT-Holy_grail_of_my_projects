// Janšta Jakub (xjanst02)

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdarg.h>
#include <string.h>
#include "util.h"
#include "queue.h"



//#define DEBUG
#ifdef DEBUG
#define output_file stdout
#else
#define output_file LOG_FILE 
#endif



FILE* LOG_FILE;
unsigned* LOG_LINE;
sem_t* log_mutex;

int* IS_OPEN;
sem_t* is_open_mutex;

sem_t** SERVICE_QUEUE_SEMAPHORES;
sem_t** SERVICE_QUEUE_MUTEXES;
queue_t** SERVICE_QUEUES; // Array of FIFO queues
sem_t** CUSTOMER_WAIT_MUTEXES;


int open_log_file(){
    LOG_FILE = fopen("proj2.out", "w");
    if (LOG_FILE == NULL){
        fprintf(stderr, "Could not open log file 'proj2.out'. Aborting...\n");
        return 1;
    }
    return 0;
}


void f_log(char* format, ...){
    va_list argv;
    va_start(argv, format);

    sem_wait(log_mutex);
    // do logging stuff
    fprintf(output_file, "%d: ", (*LOG_LINE)++);
    vfprintf(output_file, format, argv);
    fprintf(output_file, "\n");
    fflush(output_file); // dont forget to flush the output so it does not become desynchronized
    sem_post(log_mutex);

    va_end(argv);
}


int create_shared_variables(unsigned NZ){
    //
    // THERE WAS NO RULE AGAIST THE USE OF GOTO SO YOU CANNOT STOP ME!!! MUAHAHAHAHAHA!!!
    //

    // logging
    log_mutex = create_shared_semaphore(1);
    if (log_mutex == NULL) goto return_right_away;
    
    LOG_LINE = mmap(NULL, sizeof(unsigned), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (LOG_LINE == MAP_FAILED) goto return_right_away;
    *LOG_LINE = 1;

    // is_open
    is_open_mutex = create_shared_semaphore(1);
    if (is_open_mutex == NULL) goto free_log_line_and_previous;
    
    IS_OPEN = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (IS_OPEN == MAP_FAILED) goto free_is_open_mutex_and_previous;
    *IS_OPEN = 1;

    // queues
    sem_t* semaphores[] = {create_shared_semaphore(0), create_shared_semaphore(0), create_shared_semaphore(0)};
    sem_t* mutexes[] = {create_shared_semaphore(1), create_shared_semaphore(1), create_shared_semaphore(1)};
    queue_t* queues[] = {create_shared_queue(NZ), create_shared_queue(NZ), create_shared_queue(NZ)};
    
    SERVICE_QUEUE_SEMAPHORES = mmap(NULL, sizeof(sem_t*) * 3, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (SERVICE_QUEUE_SEMAPHORES == MAP_FAILED) goto free_is_open_and_previous;
    SERVICE_QUEUE_MUTEXES= mmap(NULL, sizeof(sem_t*) * 3, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (SERVICE_QUEUE_MUTEXES == MAP_FAILED) goto free_service_queue_semaphores_and_previous;
    SERVICE_QUEUES = mmap(NULL, sizeof(queue_t*) * 3, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (SERVICE_QUEUES == MAP_FAILED) goto free_service_queue_mutexes_and_previous;
    
    memcpy(SERVICE_QUEUE_SEMAPHORES, semaphores, sizeof(semaphores));
    memcpy(SERVICE_QUEUE_MUTEXES, mutexes, sizeof(mutexes));
    memcpy(SERVICE_QUEUES, queues, sizeof(queues));

    // customer_wait_mutexes
    CUSTOMER_WAIT_MUTEXES = mmap(NULL, sizeof(sem_t*) * NZ, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0); 
    if (CUSTOMER_WAIT_MUTEXES == MAP_FAILED && NZ) goto free_service_queues_and_previous;
    for (int i = 0; i < NZ; i++){
        CUSTOMER_WAIT_MUTEXES[i] = create_shared_semaphore(0);
        if (CUSTOMER_WAIT_MUTEXES[i] == NULL){
            for(int j = 0; j < i; j++){
                free_shared_semaphore(CUSTOMER_WAIT_MUTEXES[j]);
            }
            goto free_customer_wait_mutexes_and_previous;
        }
    }

    return 0;

    // look at this beauty
    free_customer_wait_mutexes_and_previous:
        munmap(CUSTOMER_WAIT_MUTEXES, sizeof(sem_t*) * NZ);

    free_service_queues_and_previous:
        munmap(SERVICE_QUEUES, sizeof(queue_t*) * 3);

    free_service_queue_mutexes_and_previous:
        munmap(SERVICE_QUEUE_MUTEXES, sizeof(sem_t*) * 3);

    free_service_queue_semaphores_and_previous:
        munmap(SERVICE_QUEUE_SEMAPHORES, sizeof(sem_t*) * 3);

    free_is_open_and_previous:
        munmap(IS_OPEN, sizeof(int));

    free_is_open_mutex_and_previous:
        free_shared_semaphore(is_open_mutex);

    free_log_line_and_previous:
        munmap(LOG_LINE, sizeof(unsigned));

    return_right_away:
        fclose(LOG_FILE);
        return 1;
}


void cleanup_shared_variables(unsigned NZ){
    // logging
    free_shared_semaphore(log_mutex);
    munmap(LOG_LINE, sizeof(unsigned));
    fclose(LOG_FILE);

    // is_open
    free_shared_semaphore(is_open_mutex);
    munmap(IS_OPEN, sizeof(int));

    // queues
    for (int i = 0; i < 3; i++){
        free_shared_semaphore(SERVICE_QUEUE_SEMAPHORES[i]);
        free_shared_semaphore(SERVICE_QUEUE_MUTEXES[i]);
        if (NZ) free_shared_queue(SERVICE_QUEUES[i]);
    }


    // customer_wait_mutexes
    for (int i = 0; i < NZ; i++){
        free_shared_semaphore(CUSTOMER_WAIT_MUTEXES[i]);
    }
    if (NZ) munmap(CUSTOMER_WAIT_MUTEXES, sizeof(sem_t*) * NZ);
}


void customer_routine(unsigned id, unsigned TZ){
    // The addition of id is necessary as the processes are created at the same time so some seed variation is needed
    srand(id + time(0));

    f_log("Z %d: started", id);
    usleep(randi(0, TZ)); // sleep for <0;TZ> sec

    sem_wait(is_open_mutex);
    int open = *IS_OPEN;
    int postet_is_open_mutex = 0;

    if (open){
        int desired_service = randi(0,2);
        f_log("Z %d: entering office for a service %d", id, desired_service + 1);

        sem_wait(SERVICE_QUEUE_MUTEXES[desired_service]);
        queue_append(SERVICE_QUEUES[desired_service], CUSTOMER_WAIT_MUTEXES[id-1]);
        sem_post(SERVICE_QUEUE_MUTEXES[desired_service]);
        
        sem_post(SERVICE_QUEUE_SEMAPHORES[desired_service]); // signal that another customer is waiting
        
        sem_post(is_open_mutex); // post here because the office could close before the customer enters the queue
        postet_is_open_mutex = 1;

        sem_wait(CUSTOMER_WAIT_MUTEXES[id-1]); // wait for the semaphore to be posted by a worker

        f_log("Z %d: called by office worker", id);
        usleep(randi(0, 10)); // sleep for <0;10> sec
    }

    if(!postet_is_open_mutex){ // ensure that the mutex is freed
        sem_post(is_open_mutex);
    }

    f_log("Z %d: going home", id);
    exit(0);
}



int worker_serve(unsigned id, unsigned TU){
    // shuffle the service indexes so that they are picked at random and each is picked at most once
    int service_indexes[3] = {0, 1, 2};
    shuffle(service_indexes, 3);

    sem_wait(is_open_mutex); // lock the open state as it could change in the process

    for (int i = 0; i < 3; i++){
        if(sem_trywait(SERVICE_QUEUE_SEMAPHORES[service_indexes[i]])){
            continue; // semaphore acquire failed
        }
        f_log("U %d: serving a service of type %d", id, service_indexes[i] + 1);

        // get the first customer mutex in the desired line
        sem_wait(SERVICE_QUEUE_MUTEXES[service_indexes[i]]);
        sem_post(queue_pop(SERVICE_QUEUES[service_indexes[i]]));
        sem_post(SERVICE_QUEUE_MUTEXES[service_indexes[i]]);

        usleep(randi(0, 10) ); // sleep for <0;10> sec
        f_log("U %d: service finished", id);
        sem_post(is_open_mutex);
        return 1;
    }
    // Nobody is waiting
    int open = *IS_OPEN;
    sem_post(is_open_mutex);

    if (open){
        f_log("U %d: taking break", id);
        usleep(randi(0, TU) ); // sleep for <0; TU> sec
        f_log("U %d: break finished", id);
        return 1;
    }
    // Closed and nobody is waiting
    // Time to go home and open up a bottle of ice cold beer after a long day
    return 0;
}


void worker_routine(unsigned id, unsigned TU){
    // The addition of id is necessary as the processes are created at the same time so some seed variation is needed
    srand(id + time(0));

    f_log("U %d: started", id);

    while(worker_serve(id, TU)){
        /*
        int result = worker_serve(id, TU);
        if (result){
            continue;
        }
        break;
        */
    }

    f_log("U %d: going home", id);
    exit(0);
}


void create_customers(unsigned NZ, unsigned TZ){
    for (int i = 0; i < NZ; i++){
        int pid = fork();
        if (pid == 0){ // child process
            customer_routine(i+1, TZ);
        }
    }
}


void create_workers(unsigned NU, unsigned TU){
    for (int i = 0; i < NU; i++){
        int pid = fork();
        if (pid == 0){ // child process
            worker_routine(i+1, TU);
        }
    }
}


int main(int argc, char* argv[]){
    int NZ, NU, TZ, TU, F;
    if (get_args(argc, argv, &NZ, &NU, &TZ, &TU, &F)){
        return 1; // stderr was already handled by get_args() so just return if an error occured
    }

    if (open_log_file()){
        return 1; // stderr was already handled by open_log_file() so just return if an error occured
    }

    if (create_shared_variables(NZ)){
        fprintf(stderr, "An error occured while creating shared variables. Aborting...\n");
        return 1; // Everything vas freed (I checked with valgrind, so at least I hope it was)
    }

    create_workers(NU, TU);
	create_customers(NZ, TZ);

    usleep(randi(F/2, F)); // sleep for <F/2; F> sec

    sem_wait(is_open_mutex);
    *IS_OPEN = 0;
    f_log("closing");
    sem_post(is_open_mutex);

    while(wait(NULL) > 0); // wait for all child processes to finish

    cleanup_shared_variables(NZ);

    return 0;
}
