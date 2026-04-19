
#ifndef TRANSPORT_TRUCKS_H
#define TRANSPORT_TRUCKS_H

struct TruckArgs {
    int truck_id;
};

void *truck_thread(void *arg);

#endif