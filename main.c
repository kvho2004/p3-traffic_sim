#include <pthread.h>      // threads, mutexes, condition variables
#include <stdio.h>        // printf for logging
#include <stdlib.h>       // exit, malloc
#include <unistd.h>       // usleep (microsecond sleep)
#include <stdatomic.h>    // atomic_bool (C11) for flags

void* f1_department(void* arg) {
    // production loop
    return NULL;
}

typedef struct {
    int factory_id;
    int dept_id;
} DeptArgs;

DeptArgs args = {3, 1};
pthread_t t;
pthread_create(&t, NULL, f3_department, (void*)&args);

// Inside the thread function:
void* f3_department(void* arg) {
    DeptArgs* a = (DeptArgs*)arg;
    printf("F%d Dept %d starting\n", a->factory_id, a->dept_id);
    return NULL;
}