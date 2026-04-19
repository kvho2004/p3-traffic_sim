#include "factory_f3.h"
#include "common.h"
#include "logger.h"
#include <unistd.h>
#include <stdio.h>


void* f3_dpt(void *arg) {
    struct F3DeptArgs*a = (struct F3DeptArgs*)arg;
    struct Storage* p1_s = &sim.f3_input.p1;
    struct Storage* p2_s = &sim.f3_input.p2;
    char label[16];

    snprintf(label, sizeof(label), "F3-DPT%d", a->dept_id);

    atomic_bool* paused = (a->dept_id == 1) ? &sim.f3_dept1_paused : &sim.f3_dept2_paused;

    while (atomic_load(&sim.running)) {
        // check if paused
        if (atomic_load(paused)) {
            usleep(50000); // sleep for 100ms to avoid busy waiting
        }

        // consume 2 parts from p1 and 1 part from p2
        // lock both storages to avoid deadlock, always lock p1 before p2


        pthread_mutex_lock(&p1_s->mtx);

        // wait until there are at least 2 units of P1 and 1 unit of P2 available
        // P3 unit = 2 P1 + 1 P2
        while (p1_s->count < 2) {
            pthread_cond_wait(&p1_s->not_empty, &p1_s->mtx);
        }

        if (!atomic_load(&sim.running)) {
            pthread_mutex_unlock(&p1_s->mtx);
            break;
        }

        // unload
        p1_s->count -= 2;

        // consume 1 unit P2 to department
        pthread_mutex_lock(&p2_s->mtx);
        while (p2_s->count < 1 ) {
            pthread_cond_wait(&p2_s->not_empty, &p2_s->mtx);
        }
        p2_s->count -= 1;
        pthread_cond_broadcast(&p2_s->not_full);
        pthread_mutex_unlock(&p2_s->mtx);

        // produce P3 unit
        usleep(F3_PRODUCE_MS * 1000); // produce time delay
        int total = atomic_fetch_add(&sim.f3_p3_count, 1) + 1; // increment total count of P3 produced  
        log_fmt(label, "Produced P3 unit %d (consumed 2 P1 and 1 P2)", total);
 
    }

    return NULL;

};