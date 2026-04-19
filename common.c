#include "common.h"

struct SimState sim;


void storage_init(struct Storage *s, int max) {
    s->count = 0;
    s->max = max;
    
    pthread_mutex_init(&s->mtx, NULL);
    pthread_cond_init(&s->not_full, NULL);
    pthread_cond_init(&s->not_empty, NULL);
    // return false
}

// objects become uninitialized

void storage_destroy(struct Storage* s) {
    pthread_mutex_destroy(&s->mtx);
    pthread_cond_destroy(&s->not_full);
    pthread_cond_destroy(&s->not_empty);
}