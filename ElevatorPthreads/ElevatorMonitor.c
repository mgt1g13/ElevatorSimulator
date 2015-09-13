//
//  ElevatorMonitor.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#include "ElevatorMonitor.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>




struct monitor{
    pthread_mutex_t monitorGlobalLock;
    pthread_cond_t * floorConditions;
    
    int currentFloor;
    int capacity;

    
};



ElevatorMonitor* new_elevator_monitor(int capacity){
    
    ElevatorMonitor *new_monitor = (ElevatorMonitor*)malloc(sizeof(ElevatorMonitor));
    
    new_monitor->capacity = capacity;
    pthread_mutex_init(&(new_monitor->monitorGlobalLock), NULL);

    new_monitor->floorConditions = (pthread_cond_t*)malloc(capacity*sizeof(pthread_cond_t));
    
    for (int i = 0; i < capacity; i++) {
        pthread_cond_init(new_monitor->floorConditions + i, NULL);
    }
    
    new_monitor->currentFloor = 0;
    
    
    
    return new_monitor;
}


