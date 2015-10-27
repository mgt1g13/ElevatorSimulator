//
//  buffer.h
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 10/10/15.
//  Copyright © 2015 Matheus Trindade. All rights reserved.
//

#ifndef buffer_h
#define buffer_h

#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

struct op{
    int thread;
    long long timestamp;
    char op;
    int floor;
    
};

struct buffer_t{
    struct op *ops;
    int n_ops;
};

typedef struct buffer_t buffer;

typedef struct op op;

//Aloca buffer
buffer* new_buffer(void);

//Escreve no buffer
void buffer_write(buffer* buff, int thread, struct timespec time, char op, int floor);

#endif /* buffer_h */
