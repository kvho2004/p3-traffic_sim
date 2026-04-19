#include "factory_f2.h"
#include "common.h"
#include "logger.h"
#include <unistd.h>
#include <time.h>

void *f2_dpt(void *arg)
{
    (void)arg;
    struct Storage *s = &sim.f2_output;

    while (atomic_load(&sim.running))
    {

        // [IMPORTANT]: implement p1 and p2 prod delays, 1 F3 dpt out of order several seconds
        while (atomic_load(&sim.f2_paused) && atomic_load(&sim.running))
        {
            struct timespec ts = {0, 50000L * 1000L};
            nanosleep(&ts, NULL);
        }

        long usec = F2_PRODUCE_MS * 2000L;
        struct timespec ts = {usec / 1000000L, (usec % 1000000L) * 1000L};
        nanosleep(&ts, NULL); /* produce for 200ms */

        pthread_mutex_lock(&s->mtx);
        while (s->count >= s->max && atomic_load(&sim.running))
            pthread_cond_wait(&s->not_full, &s->mtx);

        if (!atomic_load(&sim.running))
        {
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