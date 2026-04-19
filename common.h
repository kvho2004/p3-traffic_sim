#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stdatomic.h>

#define F1_PRODUCE_MS 100
#define F1_STORAGE_MAX 10

#define F2_PRODUCE_MS 200
#define F2_STORAGE_MAX 5

#define F3_PRODUCE_MS 400
#define F3_P1_STORAGE_MAX 10
#define F3_P2_STORAGE_MAX 5

#define TRAVEL_MS 250

#define SHIP_CAPACITY 5
#define TRUCK_CAPACITY 2
#define TRUCK_COUNT 5

// storage buffer

struct Storage {
    int count;
    int max;
    pthread_mutex_t mtx;
    pthread_cond_t not_full; // thread locks for critical condtions; avoid race condtions
    pthread_cond_t not_empty;
};

struct F3 {
    struct Storage p1;
    struct Storage p2;
};

/*
 * LOCK ORDERING RULE — must always be acquired in this order:
 *   1. f1_out.mtx
 *   2. f2_out.mtx
 *   3. f3_in.p1.mtx
 *   4. f3_in.p2.mtx
 *   5. log_mtx
 *
 * Never acquire a lower-numbered lock while holding a higher-numbered one.
 * This prevents circular waits (deadlock condition).
 */

// [IMPORTANT]: implement p1 and p2 prod delays, 1 F3 dpt out of order several seconds
struct SimState {
    struct Storage f1_output;
    struct Storage f2_output;
    struct F3 f3_input;
    atomic_int f3_p3_count;

    atomic_bool running;

    atomic_bool f1_paused;
    atomic_bool f2_paused;
    atomic_bool ship_paused;
    atomic_bool f3_dept1_paused;
    atomic_bool f3_dept2_paused;
};

extern struct SimState sim;

void storage_init(struct Storage* s, int max);
void storage_destroy(struct Storage* s);
void *watchdog_thread(void *arg);

#endif
