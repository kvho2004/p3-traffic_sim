/* factory_f1.c */
#include "factory_f1.h"
#include "common.h"
#include "logger.h"
#include <unistd.h>

void* f1_dpt(void* arg) {
    (void)arg;
    Storage* s = &sim.f1_output;

    while (atomic_load(&sim.running)) {

        /* handle pause */
        while (atomic_load(&sim.f1_paused) && atomic_load(&sim.running))
            usleep(50000);

        usleep(F1_PRODUCE_MS * 1000); /* produce for 100ms */

        pthread_mutex_lock(&s->mtx);
        while (s->count >= s->max && atomic_load(&sim.running))
            pthread_cond_wait(&s->not_full, &s->mtx);

        if (!atomic_load(&sim.running)) {
            pthread_mutex_unlock(&s->mtx);
            break;
        }

        s->count++;
        log_fmt("F1-DEPT", "Produced P1. Storage: %d/%d", s->count, s->max);
        pthread_cond_broadcast(&s->not_empty);
        pthread_mutex_unlock(&s->mtx);
    }
    return NULL;
}