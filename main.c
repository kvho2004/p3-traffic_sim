#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>

#include "common.h"
#include "logger.h"
#include "factory_f1.h"
#include "factory_f2.h"
#include "factory_f3.h"
#include "transport_ship.h"
#include "transport_trucks.h"

void scenario_pause_f1(int seconds)
{
    log_event("SCENARIO", "Pausing F1 production...");
    atomic_store(&sim.f1_paused, 1);
    sleep(seconds);
    atomic_store(&sim.f1_paused, 0);
    log_event("SCENARIO", "Resumed F1 production.");
}

void scenario_pause_f2(int seconds)
{
    log_event("SCENARIO", "Pausing F2 production...");
    atomic_store(&sim.f2_paused, 1);
    sleep(seconds);
    atomic_store(&sim.f2_paused, 0);
    log_event("SCENARIO", "Resumed F2 production.");
}

void scenario_pause_ship(int seconds)
{
    log_event("SCENARIO", "Ship out of commission...");
    atomic_store(&sim.ship_paused, 1);
    sleep(seconds);
    atomic_store(&sim.ship_paused, 0);
    log_event("SCENARIO", "Ship resumed operation.");
}

void scenario_pause_f3_dept(int dept_id, int seconds)
{
    if (dept_id == 1)
    {
        log_event("SCENARIO", "F3 Dept 1 paused...");
        atomic_store(&sim.f3_dept1_paused, 1);
        sleep(seconds);
        atomic_store(&sim.f3_dept1_paused, 0);
        log_event("SCENARIO", "F3 Dept 1 resumed.");
    }
    else
    {
        log_event("SCENARIO", "F3 Dept 2 paused...");
        atomic_store(&sim.f3_dept2_paused, 1);
        sleep(seconds);
        atomic_store(&sim.f3_dept2_paused, 0);
        log_event("SCENARIO", "F3 Dept 2 resumed.");
    }
}


int main(void)
{
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

    for (int i = 0; i < TRUCK_COUNT; i++)
    {
        truck_args[i].truck_id = i + 1;
        pthread_create(&truck_threads[i], NULL, truck_thread, &truck_args[i]);
    }

    log_event("MAIN", "Simulation started.");

    // critical section: run scenario

    sleep(5);

    scenario_pause_f1(3);
    sleep(2);

    scenario_pause_f2(3);
    sleep(2);

    scenario_pause_ship(3);
    sleep(2);

    scenario_pause_f3_dept(1, 3);

    sleep(5);

    // end simulation

    log_event("MAIN", "Stopping simulation...");
    atomic_store(&sim.running, 0);

    pthread_cond_broadcast(&sim.f1_output.not_empty);
    pthread_cond_broadcast(&sim.f1_output.not_full);
    pthread_cond_broadcast(&sim.f2_output.not_empty);
    pthread_cond_broadcast(&sim.f2_output.not_full);
    pthread_cond_broadcast(&sim.f3_input.p1.not_empty);
    pthread_cond_broadcast(&sim.f3_input.p1.not_full);
    pthread_cond_broadcast(&sim.f3_input.p2.not_empty);
    pthread_cond_broadcast(&sim.f3_input.p2.not_full);

    pthread_join(f1_thread, NULL);
    pthread_join(f2_thread, NULL);
    pthread_join(f3_dept1_thread, NULL);
    pthread_join(f3_dept2_thread, NULL);
    pthread_join(ship_thread_id, NULL);

    for (int i = 0; i < TRUCK_COUNT; i++)
    {
        pthread_join(truck_threads[i], NULL);
    }

    storage_destroy(&sim.f1_output);
    storage_destroy(&sim.f2_output);
    storage_destroy(&sim.f3_input.p1);
    storage_destroy(&sim.f3_input.p2);

    printf("\nSimulation ended. Total P3 produced: %d\n",
           atomic_load(&sim.f3_p3_count));

    return 0;
}