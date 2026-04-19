#include "transport_ship.h"
#include "common.h"
#include "logger.h"
#include <unistd.h>

void *ship_thread(void *arg) {

    (void)arg; // unused parameter

    char label[16];

    struct Storage* f1src = &sim.f1_output;
    struct Storage* f3dest = &sim.f3_input.p1; // F3 input storage (P1)
    
    snprintf(label, sizeof(label), "SHIP");


    while (atomic_load(&sim.running)) {
        // check if paused
        if (atomic_load(&sim.ship_paused) && atomic_load(&sim.running)) {
            usleep(50000); // sleep for 100ms to avoid busy waiting
            continue;
        }


        int loaded = 0;


        // load from F1 output storage, 1 unit at a time until ship capacity or F1 storage is empty
        pthread_mutex_lock(&f1src->mtx);

        // wait until there is at least 1 unit of P1 available
        while (f1src->count < 1) {
            pthread_cond_wait(&f1src->not_empty, &f1src->mtx);
        }

        if (!atomic_load(&sim.running)) {
            pthread_mutex_unlock(&f1src->mtx);
            break;
        }

        loaded = (f1src->count < SHIP_CAPACITY) ? f1src->count : SHIP_CAPACITY; // load as much as possible up to ship capacity
        f1src->count -= loaded;
        pthread_cond_broadcast(&f1src->not_full);
        pthread_mutex_unlock(&f1src->mtx);

        log_fmt("SHIP", "Loaded %d P1. Travelling to F3...", loaded);
        usleep(TRAVEL_MS * 1000); // simulate travel time

        // unload to F3 input storage (P1), 1 unit at a time until ship is empty or F3 storage is full
        pthread_mutex_lock(&f3dest->mtx);
        while(f3dest->count >= f3dest->max) {
            pthread_cond_wait(&f3dest->not_full, &f3dest->mtx);
        }
        f3dest->count += loaded;
        pthread_cond_broadcast(&f3dest->not_full);
        pthread_mutex_unlock(&f3dest->mtx);
        log_fmt("SHIP", "Unloaded %d P1 to F3. Returning to F1...", loaded);
        usleep(TRAVEL_MS * 1000); // simulate travel time      
    }

    return NULL;
};