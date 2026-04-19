#include <pthread.h>      // threads, mutexes, condition variables
#include <stdio.h>        // printf for logging
#include <stdlib.h>       // exit, malloc
#include <unistd.h>       // usleep (microsecond sleep)
#include <stdatomic.h>    // atomic_bool (C11) for flags

#include "common.h"
#include "logger.h"
#include "factory_f1.h"
#include "factory_f2.h"
#include "factory_f3.h"
#include "transport_ship.h"
#include "transport_trucks.h"


int main(void) {
    // initialize simulation state
    storage_init(&sim.f1_output, F1_STORAGE_MAX);
    storage_init(&sim.f2_output, F2_STORAGE_MAX);
    storage_init(&sim.f3_input.p1, F3_P1_STORAGE_MAX);
    storage_init(&sim.f3_input.p2, F3_P2_STORAGE_MAX);
    atomic_init(&sim.f3_p3_count, 0);

    atomic_store(&sim.running, 1);
    atomic_store(&sim.f1_paused, 0);
    atomic_store(&sim.f2_paused, 0);
    atomic_store(&sim.ship_paused, 0);
    atomic_store(&sim.f3_dept1_paused, 0);
    atomic_store(&sim.f3_dept2_paused, 0);

    // create threads
    pthread_t f1_thread, f2_thread, f3_dept1_thread, f3_dept2_thread, ship_thread_id;
    pthread_t truck_threads[TRUCK_COUNT];
    static struct TruckArgs truck_args[TRUCK_COUNT];
    static struct F3DeptArgs dept_args[2] = {{1}, {2}};

    pthread_create(&f1_thread, NULL, f1_dpt, NULL);
    pthread_create(&f2_thread, NULL, f2_dpt, NULL);
    pthread_create(&f3_dept1_thread, NULL, f3_dpt, &dept_args[0]);
    pthread_create(&f3_dept2_thread, NULL, f3_dpt, &dept_args[1]);
    pthread_create(&ship_thread_id, NULL, ship_thread, NULL);

    for (int i = 0; i < TRUCK_COUNT; i++) {
        truck_args[i].truck_id = i + 1;
        pthread_create(&truck_threads[i], NULL, truck_thread, &truck_args[i]);
    }

    log_event("MAIN", "Simulation started.");

    sleep(10); // run simulation for 10 seconds

    // spec: The P1 production stops for several seconds.
    // The P2 production stops for several seconds.
    // The ship is out of commission for several seconds.
    // One F3 department is out of order for several seconds.

    log_event("MAIN", "Pausing F1 production for 3 seconds...");
    atomic_store(&sim.f1_paused, 1);
    sleep(3);
    atomic_store(&sim.f1_paused, 0);
    log_event("MAIN", "Resumed F1 production.");

    sleep(5);

    log_event("MAIN", "Stopping simulation...");
    atomic_store(&sim.running, 0);

    // wake all blocked threads so they can see running=0 and exit

    pthread_cond_broadcast(&sim.f1_output.not_empty);
    pthread_cond_broadcast(&sim.f1_output.not_full);
    pthread_cond_broadcast(&sim.f2_output.not_empty);
    pthread_cond_broadcast(&sim.f2_output.not_full);
    pthread_cond_broadcast(&sim.f3_input.p1.not_empty);
    pthread_cond_broadcast(&sim.f3_input.p1.not_full);
    pthread_cond_broadcast(&sim.f3_input.p2.not_empty);
    pthread_cond_broadcast(&sim.f3_input.p2.not_full);

    // join threads
    pthread_join(f1_thread, NULL);
    pthread_join(f2_thread, NULL);
    pthread_join(f3_dept1_thread, NULL);
    pthread_join(f3_dept2_thread, NULL);
    pthread_join(ship_thread_id, NULL);
    for (int i = 0; i < TRUCK_COUNT; i++) {
        pthread_join(truck_threads[i], NULL);
    }

    // cleanup
    storage_destroy(&sim.f1_output);
    storage_destroy(&sim.f2_output);
    storage_destroy(&sim.f3_input.p1);
    storage_destroy(&sim.f3_input.p2);

    printf("\nSimulation ended. Total P3 produced: %d\n",
           atomic_load(&sim.f3_p3_count));
    return 0;
};