#include "factory_f2.h"
#include "common.h"
#include "logger.h"
#include <unistd.h>

void* f1_dpt(void* arg) {
    (void)arg;
    Storage* s = &sim.f2_output;

    while (atomic_load(&sim.running)) {

        // [IMPORTANT]: implement p1 and p2 prod delays, 1 F3 dpt out of order several seconds
        while (atomic_load(&sim.f2_paused) && atomic_load(&sim.running))
            usleep(50000);

        usleep(F2_PRODUCE_MS * 2000); /* produce for 200ms */

        pthread_mutex_lock(&s->mtx);
        while (s->count >= s->max && atomic_load(&sim.running))
            pthread_cond_wait(&s->not_full, &s->mtx);

        if (!atomic_load(&sim.running)) {
            pthread_mutex_unlock(&s->mtx);
            break;
        }

        s->count++;
        log_fmt("F2-DEPT", "Produced P2. Storage: %d/%d", s->count, s->max);
        pthread_cond_broadcast(&s->not_empty);
        pthread_mutex_unlock(&s->mtx);
    }
    return NULL;
}