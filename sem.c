/*
 * sem-producer-consumer.c  - demonstrates a basic producer-consumer
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define NUM_LOOPS 20

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun already defined */
#else
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};
#endif

int main() {
    int sem_set_id;
    union semun sem_val;
    int child_pid;
    int i;
    struct sembuf sem_op;
    struct timespec delay;

    // Create a private semaphore set (1 semaphore)
    sem_set_id = semget(IPC_PRIVATE, 1, 0600);
    if (sem_set_id == -1) {
        perror("semget");
        exit(1);
    }

    printf("Semaphore set created with ID: %d\n", sem_set_id);

    // Initialize semaphore value = 0
    sem_val.val = 0;
    semctl(sem_set_id, 0, SETVAL, sem_val);

    // Fork a child -> consumer
    child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        exit(1);
    }
    else if (child_pid == 0) {
        // Child = consumer
        for (i = 0; i < NUM_LOOPS; i++) {
            sem_op.sem_num = 0;
            sem_op.sem_op = -1;   // wait
            sem_op.sem_flg = 0;
            semop(sem_set_id, &sem_op, 1);

            printf("consumer: %d\n", i);
            fflush(stdout);
        }
    }
    else {
        // Parent = producer
        for (i = 0; i < NUM_LOOPS; i++) {
            printf("producer: %d\n", i);
            fflush(stdout);

            sem_op.sem_num = 0;
            sem_op.sem_op = 1;   // signal
            sem_op.sem_flg = 0;
            semop(sem_set_id, &sem_op, 1);

            if (rand() > 3 * (RAND_MAX / 4)) {
                delay.tv_sec = 0;
                delay.tv_nsec = 10000000;
                nanosleep(&delay, NULL);
            }
        }

        // Remove semaphore
        semctl(sem_set_id, 0, IPC_RMID, sem_val);
    }

    return 0;
}
