//
//  buffer.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 10/10/15.
//  Copyright © 2015 Matheus Trindade. All rights reserved.
//

#include "buffer.h"
#include <stdlib.h>
//#include <time.h>
#include <stdio.h>


#define TIME_FACTOR (1000000)

struct buffer_t{
    
    char buff[400][100];
    int position;
};


buffer* new_buffer(void){
    buffer* new_buffer = (buffer*)malloc(sizeof(buffer));
    new_buffer->position = 0;
    
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
    
    printf("%d %lli %c %d\n", thread, (long long)((now.tv_sec-base.tv_sec)*1e7)+ now.tv_nsec - base.tv_nsec, op, floor);


    //buff->position++;
}