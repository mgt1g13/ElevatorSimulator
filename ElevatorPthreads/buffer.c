//
//  buffer.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 10/10/15.
//  Copyright © 2015 Matheus Trindade. All rights reserved.
//

#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>


#define TIME_FACTOR (1000000)

//struct op{
//    int thread;
//    long long timestamp;
//    char op;
//    int floor;
//    
//};
//
//struct buffer_t{
//    struct op ops[500];
//    int n_ops;
//};




buffer* new_buffer(int max_op){
    buffer* new_buffer = (buffer*)malloc(sizeof(buffer));
    new_buffer->ops = (op*)malloc(sizeof(op)*max_op);
    new_buffer->n_ops = 0;
    
    return new_buffer;
}


void buffer_write(buffer* buff, int thread, struct timespec base, char op, int floor){
    struct timespec now;

#ifdef __MACH__ // OS X
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    now.tv_sec = mts.tv_sec;
    now.tv_nsec = mts.tv_nsec;
    
#else
    clock_gettime(CLOCK_REALTIME, &now);
#endif
    
//    printf("%d %lli %c %d\n", thread, (long long)((now.tv_sec-base.tv_sec)*1e7)+ now.tv_nsec - base.tv_nsec, op, floor);
    
    buff->ops[buff->n_ops].thread = thread;
//    buff->ops[buff->n_ops].timestamp = ((long long)((now.tv_sec-base.tv_sec)*1e6)+ (now.tv_nsec/1000 - base.tv_nsec/1000));
     buff->ops[buff->n_ops].timestamp = ((long long)((now.tv_sec-base.tv_sec)*1e9)+ (now.tv_nsec - base.tv_nsec));
//    buff->ops[buff->n_ops].timestamp = ((now.tv_sec-base.tv_sec)*1e9)+ (now.tv_nsec - base.tv_nsec);
    buff->ops[buff->n_ops].op = op;
    buff->ops[buff->n_ops].floor = floor;
    buff->n_ops++;

}