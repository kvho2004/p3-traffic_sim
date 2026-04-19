/* watchdog.c */
#include "common.h"
#include "logger.h"
#include <unistd.h>

void *watchdog_thread(void *arg)
{
    (void)arg;
    int prev_p3 = 0;
    int stall_count = 0;

    while (atomic_load(&sim.running))
    {
        sleep(2); /* check every 2 seconds */

        int curr_p3 = atomic_load(&sim.f3_p3_count);

        /* snapshot storage counts — lock each briefly */
        pthread_mutex_lock(&sim.f1_output.mtx);
        int f1 = sim.f1_output.count;
        pthread_mutex_unlock(&sim.f1_output.mtx);

        pthread_mutex_lock(&sim.f2_output.mtx);
        int f2 = sim.f2_output.count;
        pthread_mutex_unlock(&sim.f2_output.mtx);

        pthread_mutex_lock(&sim.f3_input.p1.mtx);
        int f3p1 = sim.f3_input.p1.count;
        pthread_mutex_unlock(&sim.f3_input.p1.mtx);

        pthread_mutex_lock(&sim.f3_input.p2.mtx);
        int f3p2 = sim.f3_input.p2.count;
        pthread_mutex_unlock(&sim.f3_input.p2.mtx);

        log_fmt("WATCHDOG",
                "F1=%d/10  F2=%d/5  F3-P1=%d/10  F3-P2=%d/5  P3-total=%d",
                f1, f2, f3p1, f3p2, curr_p3);

        if (curr_p3 == prev_p3)
        {
            stall_count++;
            if (stall_count >= 3) /* no P3 for 6 seconds */
                log_fmt("WATCHDOG", "*** WARNING: P3 production stalled! ***");
        }
        else
        {
            stall_count = 0;
        }
        prev_p3 = curr_p3;
    }
    return NULL;
}