#include "transport_trucks.h"
#include "common.h"
#include "logger.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>

void *truck_thread(void *arg)
{
    struct TruckArgs *t = (struct TruckArgs *)arg;
    struct Storage *f2src = &sim.f2_output;
    struct Storage *f3dest = &sim.f3_input.p2; // F3 input storage (P2)
    char label[16];

    snprintf(label, sizeof(label), "TRUCK%d", t->truck_id);

    while (atomic_load(&sim.running))
    {
        pthread_mutex_lock(&f2src->mtx);

        while (f2src->count < TRUCK_CAPACITY && atomic_load(&sim.running))
        {
            pthread_cond_wait(&f2src->not_empty, &f2src->mtx);
        }
        if (!atomic_load(&sim.running))
        {
            pthread_mutex_unlock(&f2src->mtx);
            break;
        }

        f2src->count -= TRUCK_CAPACITY;
        pthread_cond_broadcast(&f2src->not_full);
        pthread_mutex_unlock(&f2src->mtx);

        log_fmt(label, "Loaded %d P2. Travelling to F3...", TRUCK_CAPACITY);
        long usec = TRAVEL_MS * 1000L;
        struct timespec ts = {usec / 1000000L, (usec % 1000000L) * 1000L};
        nanosleep(&ts, NULL); // simulate travel time

        pthread_mutex_lock(&f3dest->mtx);
        while (f3dest->count + TRUCK_CAPACITY > f3dest->max && atomic_load(&sim.running))
        {
            pthread_cond_wait(&f3dest->not_full, &f3dest->mtx);
        }
        if (!atomic_load(&sim.running))
        {
            pthread_mutex_unlock(&f3dest->mtx);
            break;
        }

        f3dest->count += TRUCK_CAPACITY;
        pthread_cond_broadcast(&f3dest->not_full);
        pthread_mutex_unlock(&f3dest->mtx);

        log_fmt(label, "Unloaded %d P2 to F3. Returning to F2...", TRUCK_CAPACITY);
        usec = TRAVEL_MS * 1000L;
        ts = (struct timespec){usec / 1000000L, (usec % 1000000L) * 1000L};
        nanosleep(&ts, NULL); // simulate travel time
    }
    return NULL;
};