//
//  ElevatorMonitor.h
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#ifndef __ElevatorPthreads__ElevatorMonitor__
#define __ElevatorPthreads__ElevatorMonitor__

#include <stdio.h>

#define DOOR_OPEN 0
#define DOOR_CLOSED 1

#define UP 1
#define DOWN -1
#define STILL 0

typedef struct monitor ElevatorMonitor;

typedef int direction;

ElevatorMonitor* new_elevator_monitor(int capacity, int numberOfFloors, int number_of_clients);


#endif /* defined(__ElevatorPthreads__ElevatorMonitor__) */
