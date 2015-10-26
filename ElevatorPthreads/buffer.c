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


void buffer_write(buffer* buff, int thread, time_t base, char op, int floor){
    time_t now;
    time(&now);
    printf("%.10lf\n", difftime(now, base));
    printf("%d %d %c %d\n", thread, (int)(difftime(now, base)*TIME_FACTOR), op, floor);
//    printf("%d %f %c %d\n", thread, difftime(now, base)*TIME_FACTOR, op, floor);
    //buff->position++;
}