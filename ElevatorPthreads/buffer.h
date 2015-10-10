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

typedef struct buffer_t buffer;

void buffer_write(buffer* buff, int thread, time_t time, char op, int floor);

#endif /* buffer_h */
