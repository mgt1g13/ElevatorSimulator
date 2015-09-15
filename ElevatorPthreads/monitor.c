//
//  ElevatorMonitor.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

//#include "monitor.h"
#include "monitorElevatorSide.h"
#include "monitorUserSide.h"
#include "insidePanel.h"
#include "outsidePanel.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define DOOR_OPEN 0
#define DOOR_CLOSED 1

#define GOING_UP 1
#define GOING_DOWN -1
#define STILL 0


struct monitor{
    pthread_mutex_t monitorGlobalLock;
    pthread_cond_t timedWait;
    pthread_cond_t *floorConditions;
    
    insidePanel *inside_panel;
    outsidePanel **outside_panels;
    
    int doorState;
    int currentFloor;
    int movementState;
    int capacity;
    int numberOfFloors;
    
};



ElevatorMonitor* new_elevator_monitor(int capacity, int numberOfFloors){
    
    ElevatorMonitor *new_monitor = (ElevatorMonitor*)malloc(sizeof(ElevatorMonitor));
   
    new_monitor->numberOfFloors = numberOfFloors;
    new_monitor->capacity = capacity;
    new_monitor->doorState  = DOOR_OPEN;
    new_monitor->currentFloor = TERREO;
    
    
    //Mutex init
    pthread_mutex_init(&(new_monitor->monitorGlobalLock), NULL);

    //Condition variables init
    new_monitor->floorConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    for (int i = 0; i < numberOfFloors; i++) {
        pthread_cond_init(new_monitor->floorConditions + i, NULL);
    }
    
    
    
    new_monitor->inside_panel = new_insidePanel(numberOfFloors);
    
    new_monitor->outside_panels = (outsidePanel**)malloc(numberOfFloors*sizeof(outsidePanel*));
    for (int i = 0; i < numberOfFloors ; i++) {
        new_monitor->outside_panels[i] = new_outsidePanel();
    }
    
    
    
    
    
    
    
    return new_monitor;
}

void elevator_close_doors(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    monitor->doorState = DOOR_CLOSED;
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}

void elevator_open_doors(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    monitor->doorState = DOOR_OPEN;
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}


//Floor to go or -1 if should not go any where
int _get_next_floor(ElevatorMonitor* monitor){
    if (monitor->movementState == GOING_UP) {
        //Check up
        for (int i = monitor->currentFloor +1; i < monitor->numberOfFloors; i++) {
            if (insidePanel_is_button_on(monitor->inside_panel, i)) {
                return i;
            }
        }
        //Check down
        for (int i = monitor->currentFloor; i >= 0 ; i--) {
            <#statements#>
        }
        
    }
    return -1;
}

void elevator_move(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}



