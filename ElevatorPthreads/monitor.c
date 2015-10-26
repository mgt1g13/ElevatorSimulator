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
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define DOOR_OPEN 0
#define DOOR_CLOSED 1

#define TERREO 0

#define UP 1
#define DOWN -1
#define STILL 0

#define SECONDS_BETWEEN_FLOORS 0
#define TIME_FOR_PEOPLE_TO_LEAVE 100000
#define PEOPLE_ENTER_TIME 100000

//#define TIME_FOR_PEOPLE_TO_LEAVE 1000
//#define PEOPLE_ENTER_TIME 1000








struct monitor{
    pthread_mutex_t monitorGlobalLock;
    pthread_cond_t timedWait;
    pthread_cond_t *floorUpConditions;
    pthread_cond_t *floorDownConditions;
    pthread_cond_t *floorLeaveConditions;
    pthread_cond_t hasFloorToGo;
    
    insidePanel *inside_panel;
    outsidePanel **outside_panels;
    
    int number_of_clients;
    int doorState;
    int currentFloor;
    direction movementState;
    int capacity;
    int numberOfFloors;
    int people_inside;
    int* destinies;
    time_t start_time;
    
};



ElevatorMonitor* new_elevator_monitor(int capacity, int numberOfFloors, int number_of_clients){
    
    ElevatorMonitor *new_monitor = (ElevatorMonitor*)malloc(sizeof(ElevatorMonitor));
   
    new_monitor->numberOfFloors = numberOfFloors;
    new_monitor->capacity = capacity;
    new_monitor->doorState  = DOOR_CLOSED;
    new_monitor->currentFloor = TERREO;
    new_monitor->people_inside = 0;
    new_monitor->movementState = UP;
    new_monitor->number_of_clients = number_of_clients;
    
    //Mutex init
    pthread_mutex_init(&(new_monitor->monitorGlobalLock), NULL);
    
    new_monitor->destinies = (int*)malloc(sizeof(int)*numberOfFloors);

    //Condition variables init
    new_monitor->floorUpConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    new_monitor->floorDownConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    new_monitor->floorLeaveConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    for (int i = 0; i < numberOfFloors; i++) {
        pthread_cond_init(new_monitor->floorUpConditions + i, NULL);
        pthread_cond_init(new_monitor->floorDownConditions + i, NULL);
        pthread_cond_init(new_monitor->floorLeaveConditions + i, NULL);
        new_monitor->destinies[i] = 0;
    }
    
    pthread_cond_init(&new_monitor->timedWait, NULL);
    pthread_cond_init(&new_monitor->hasFloorToGo, NULL);
    
    
    new_monitor->inside_panel = new_insidePanel(numberOfFloors);
    
    new_monitor->outside_panels = (outsidePanel**)malloc(numberOfFloors*sizeof(outsidePanel*));
    for (int i = 0; i < numberOfFloors ; i++) {
        new_monitor->outside_panels[i] = new_outsidePanel();
    }
    
    time(&new_monitor->start_time);
    
    return new_monitor;
}





void elevator_close_doors(ElevatorMonitor* monitor, buffer* buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    while (monitor->destinies[monitor->currentFloor] != 0) {
        pthread_mutex_unlock(&monitor->monitorGlobalLock);
        elevator_wait_on_floor(monitor);
        pthread_mutex_lock(&monitor->monitorGlobalLock);
    }

    if(monitor->doorState != DOOR_CLOSED){
        buffer_write(buff, 0,  monitor->start_time, 'F', monitor->currentFloor);
        monitor->doorState = DOOR_CLOSED;
    }
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}




void _wait(ElevatorMonitor* monitor, int seconds){
    
    struct timespec ts;
    struct timeval tp;
    
    //Set wait time
    gettimeofday(&tp, NULL);
    
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    //ts.tv_sec += seconds;
    ts.tv_nsec += seconds ;
    pthread_cond_timedwait(&monitor->timedWait, &monitor->monitorGlobalLock, &ts);
    
}



void elevator_wait_on_floor(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
//    buffer_write(NULL, 0, monitor->start_time, 'B', monitor->currentFloor);
    _wait(monitor, PEOPLE_ENTER_TIME);
//    buffer_write(NULL, 0, monitor->start_time, 'E', monitor->currentFloor);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}


void elevator_open_doors(ElevatorMonitor* monitor, buffer* buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    if (monitor->doorState != DOOR_OPEN) {
        buffer_write(buff, 0, monitor->start_time, 'A', monitor->currentFloor);
        monitor->doorState = DOOR_OPEN;
    }
    
    
    //Signals Client waiting for the elevator on the current floor
    pthread_cond_signal(monitor->floorLeaveConditions + monitor->currentFloor);
    
    
    if (insidePanel_is_button_on(monitor->inside_panel, monitor->currentFloor)) {
        buffer_write(buff, 0, monitor->start_time, 'i', monitor->currentFloor);
        
        insidePanel_turn_off_button(monitor->inside_panel, monitor->currentFloor);

    }
    
//    buffer_write(buff, 0, monitor->start_time, 'i', monitor->currentFloor);
//    
//    insidePanel_turn_off_button(monitor->inside_panel, monitor->currentFloor);

    //Wait for 1 second so clients can leave
    _wait(monitor, TIME_FOR_PEOPLE_TO_LEAVE);
    
    
    
    
    
    if(monitor->movementState == UP){

//        printf("is up button -> %d\n", outsidePanel_is_up_button_on(monitor->outside_panels[monitor->currentFloor]));
        if (outsidePanel_is_up_button_on(monitor->outside_panels[monitor->currentFloor])) {
            buffer_write(buff, 0, monitor->start_time, 's', monitor->currentFloor);
            outsidePanel_turn_off_up_button(monitor->outside_panels[monitor->currentFloor]);

        }

        //Avisa quem quiser subir
        pthread_cond_signal(monitor->floorUpConditions + monitor->currentFloor);
        
    }
    if(monitor->movementState == DOWN){
        
        if (outsidePanel_is_down_button_on(monitor->outside_panels[monitor->currentFloor])) {
            buffer_write(buff, 0, monitor->start_time, 'd', monitor->currentFloor);
            outsidePanel_turn_off_down_button(monitor->outside_panels[monitor->currentFloor]);
            
        }
        pthread_cond_signal(monitor->floorDownConditions + monitor->currentFloor);
    }
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}



int elevator_get_next_floor(ElevatorMonitor *monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
  //  printf("LA -> %d\n", monitor->currentFloor);
    while (1) {
        int full = monitor->people_inside == monitor->capacity;
        
        if (monitor->movementState == UP) {
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if ( (outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full)  || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
//            for (int i = monitor->currentFloor; i >= 0; i--) {
            for (int i = monitor->numberOfFloors-1; i > monitor->currentFloor; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i])&& !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
//            for (int i = monitor->currentFloor; i >= 0; i--) {
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            for (int i = 0; i < monitor->currentFloor; i++) {
  
                if ((outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
//            for (int i = monitor->currentFloor; i <monitor->numberOfFloors; i++) {

        }
        
        if (monitor->movementState == DOWN) {
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
//            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
            for (int i = 0; i < monitor->currentFloor; i++) {
                if ((outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if ((outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            for (int i =  monitor->numberOfFloors-1; i >monitor->currentFloor; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }

            
        }
        
        
        
        //Espera por um andar
        pthread_cond_wait(&monitor->hasFloorToGo, &monitor->monitorGlobalLock);
    }

}

int elevator_get_current_floor(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    int ret = monitor->currentFloor;
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    
    return ret;
}

direction elevator_get_current_movement_state(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    direction ret = monitor->movementState;
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    
    return ret;
    
}

void elevator_set_current_movement_state(ElevatorMonitor* monitor, direction dir){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    if (monitor->doorState == DOOR_CLOSED) {
        printf("This function should only be called with open doors");
        exit(1);
    }
    
    monitor->movementState = dir;
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    elevator_open_doors(monitor, NULL);
    elevator_wait_on_floor(monitor);
    
}

//Should be called from within a mutex block locked on the globalMonitor mutex
void _move(ElevatorMonitor* monitor, direction dir){
    
    struct timespec ts;
    struct timeval tp;
    
    //Set wait time
    gettimeofday(&tp, NULL);
    
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += SECONDS_BETWEEN_FLOORS;
    
    pthread_cond_timedwait(&monitor->timedWait, &monitor->monitorGlobalLock, &ts);
  //  printf("Dir - > %d\n", dir);
    monitor->currentFloor += dir;
//    printf("In floor %d\n", monitor->currentFloor);
    
}


void _elevator_move_between_floors(ElevatorMonitor* monitor, direction dir){
    _move(monitor, dir);
    monitor->movementState = dir;
}




void elevator_move(ElevatorMonitor* monitor, direction dir){
    pthread_mutex_lock(&monitor->monitorGlobalLock);

    if (monitor->currentFloor == monitor->numberOfFloors - 1) {
        dir = DOWN;
    }
    else if (monitor->currentFloor == 0){
        dir = UP;
    }
    

    _elevator_move_between_floors(monitor, dir);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}


int elevator_should_end(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    
    int should_end = monitor->number_of_clients?0:1;
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    return should_end;
}

void person_travel(ElevatorMonitor* monitor, int thread, int person_current_floor, int destiny, buffer* buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    int inside = 0;
    if (person_current_floor == destiny) {
        pthread_mutex_unlock(&monitor->monitorGlobalLock);
        return;
    }

    
    int dir = UP;
    if (destiny < person_current_floor) {
        dir = DOWN;
    }
    else{
        dir = UP;
    }
    if(monitor->currentFloor == person_current_floor
       && monitor->doorState == DOOR_OPEN
       && monitor->people_inside < monitor->capacity
       && monitor->movementState == dir){
        inside = 1;
    }


    
    while(!inside) {
        //TODO: Transformar em while
        if(person_current_floor < destiny){
            if (!outsidePanel_is_up_button_on(monitor->outside_panels[person_current_floor])) {
                buffer_write(buff, thread, monitor->start_time, 'S', person_current_floor);
                outsidePanel_turn_on_up_button(monitor->outside_panels[person_current_floor]);

            }
            
            pthread_cond_signal(&monitor->hasFloorToGo);
            dir = UP;

            buffer_write(buff, thread, monitor->start_time, 'B', monitor->currentFloor);
            pthread_cond_wait(monitor->floorUpConditions+person_current_floor, &monitor->monitorGlobalLock);
            buffer_write(buff, thread, monitor->start_time, 'E', monitor->currentFloor);
            
            
        }
        else if(person_current_floor > destiny){
            if (!outsidePanel_is_down_button_on(monitor->outside_panels[person_current_floor])) {
                
                buffer_write(buff, thread, monitor->start_time, 'D', person_current_floor);
                
                outsidePanel_turn_on_down_button(monitor->outside_panels[person_current_floor]);

            }
            pthread_cond_signal(&monitor->hasFloorToGo);
            dir = DOWN;
            
            buffer_write(buff, thread, monitor->start_time, 'B', monitor->currentFloor);
            pthread_cond_wait(monitor->floorDownConditions+person_current_floor, &monitor->monitorGlobalLock);
            buffer_write(buff, thread, monitor->start_time, 'E', monitor->currentFloor);
            
        }
        
        //Pode entrar?
        if(monitor->people_inside < monitor->capacity && monitor->currentFloor == person_current_floor && monitor->doorState == DOOR_OPEN){
            inside = 1;

            pthread_cond_signal(&monitor->hasFloorToGo);
            
            //Avisa para mais pessoas entrarem
            if(dir == UP){
                pthread_cond_signal(monitor->floorUpConditions+person_current_floor);
            }
            else{
                pthread_cond_signal(monitor->floorDownConditions+person_current_floor);
            }
        }else{
            _wait(monitor, (2*TIME_FOR_PEOPLE_TO_LEAVE));
        }
    }
    
    buffer_write(buff, thread, monitor->start_time, 'N', monitor->currentFloor);
    
    //Entra
    monitor->people_inside++;
    monitor->destinies[destiny]++;

    //Aperta o botao para o andar de destino
    buffer_write(buff, thread, monitor->start_time, 'I', destiny);
    insidePanel_press_button(monitor->inside_panel, destiny);

    pthread_cond_signal(&monitor->hasFloorToGo);

    //Espera chegar no andar
    pthread_cond_wait(monitor->floorLeaveConditions+destiny, &monitor->monitorGlobalLock);
    
    buffer_write(buff, thread, monitor->start_time, 'V', monitor->currentFloor);
    monitor->people_inside--;
    monitor->destinies[destiny]--;
    
    //Sai, avisa mais alguem pra sair
    pthread_cond_signal(monitor->floorLeaveConditions+destiny);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    return;
}


void person_visit(int miliseconds){
    
    usleep(miliseconds/1000);
    
}



void person_end(ElevatorMonitor* monitor, int thread, buffer *buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    monitor->number_of_clients--;
    buffer_write(buff, thread, monitor->start_time, 'M', 0);
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}

