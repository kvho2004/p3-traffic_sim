#ifndef FACTORY_F3_H
#define FACTORY_F3_H

struct F3DeptArgs{
    int dept_id;  // 1 or 2, f3 has 2 dpts
};

void* f3_dpt(void* arg);

#endif