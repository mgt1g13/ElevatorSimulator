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

#define SECONDS_BETWEEN_FLOORS 5
#define TIME_FOR_PEOPLE_TO_LEAVE 1
#define PEOPLE_ENTER_TIME 1

#define TIME_FACTOR (1/1000000000)



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
    time_t start_time;
    
};



ElevatorMonitor* new_elevator_monitor(int capacity, int numberOfFloors, int number_of_clients){
    
    ElevatorMonitor *new_monitor = (ElevatorMonitor*)malloc(sizeof(ElevatorMonitor));
   
    new_monitor->numberOfFloors = numberOfFloors;
    new_monitor->capacity = capacity;
    new_monitor->doorState  = DOOR_OPEN;
    new_monitor->currentFloor = TERREO;
    new_monitor->people_inside = 0;
    new_monitor->movementState = UP;
    new_monitor->number_of_clients = number_of_clients;
    
    //Mutex init
    pthread_mutex_init(&(new_monitor->monitorGlobalLock), NULL);

    //Condition variables init
    new_monitor->floorUpConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    new_monitor->floorDownConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    new_monitor->floorLeaveConditions = (pthread_cond_t*)malloc(numberOfFloors*sizeof(pthread_cond_t));
    for (int i = 0; i < numberOfFloors; i++) {
        pthread_cond_init(new_monitor->floorUpConditions + i, NULL);
        pthread_cond_init(new_monitor->floorDownConditions + i, NULL);
        pthread_cond_init(new_monitor->floorLeaveConditions + i, NULL);
    }
    
    pthread_cond_init(&new_monitor->timedWait, NULL);
    pthread_cond_init(&new_monitor->hasFloorToGo, NULL);
    
    
    new_monitor->inside_panel = new_insidePanel(numberOfFloors);
    
    new_monitor->outside_panels = (outsidePanel**)malloc(numberOfFloors*sizeof(outsidePanel*));
    for (int i = 0; i < numberOfFloors ; i++) {
        new_monitor->outside_panels[i] = new_outsidePanel();
//        printf("%d\n", new_monitor->outside_panels[i]);
    }
    time(&new_monitor->start_time);
    
    return new_monitor;
}





void elevator_close_doors(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    monitor->doorState = DOOR_CLOSED;
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}




void _wait(ElevatorMonitor* monitor, int seconds){
    
    struct timespec ts;
    struct timeval tp;
    
    //Set wait time
    gettimeofday(&tp, NULL);
    
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += seconds;
    
    pthread_cond_timedwait(&monitor->timedWait, &monitor->monitorGlobalLock, &ts);
    
}



void elevator_wait_on_floor(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    _wait(monitor, PEOPLE_ENTER_TIME);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}


void elevator_open_doors(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
//    static int aux = 0;
    
//    printf("Openning doors on floor -> %d\n", monitor->currentFloor);
    
    monitor->doorState = DOOR_OPEN;
    
    //Signals Client waiting for the elevator on the current floor
    pthread_cond_signal(monitor->floorLeaveConditions + monitor->currentFloor);
    
    insidePanel_turn_off_button(monitor->inside_panel, monitor->currentFloor);

    //Wait for 1 second so clients can leave
    _wait(monitor, TIME_FOR_PEOPLE_TO_LEAVE);
    
    

    
    if(monitor->movementState == UP){
        outsidePanel_turn_off_up_button(monitor->outside_panels[monitor->currentFloor]);
        //Avisa quem quiser subir
        pthread_cond_signal(monitor->floorUpConditions + monitor->currentFloor);
        
    }
    if(monitor->movementState == DOWN){
        outsidePanel_turn_off_down_button(monitor->outside_panels[monitor->currentFloor]);
        //Avisa quem quiser descer
        pthread_cond_signal(monitor->floorDownConditions + monitor->currentFloor);
    }
    

    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}



int elevator_get_next_floor(ElevatorMonitor *monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    while (1) {
        
        
        if (monitor->movementState == UP) {
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if (outsidePanel_is_up_button_on(monitor->outside_panels[i]) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                  //  printf("i -> %d -> %d\n", i,outsidePanel_is_up_button_on(monitor->outside_panels[i]));
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if (outsidePanel_is_down_button_on(monitor->outside_panels[i]) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if (outsidePanel_is_up_button_on(monitor->outside_panels[i]) || insidePanel_is_button_on(monitor->inside_panel, i)) {
//                    printf("i -> %d -> %d\n", i,outsidePanel_is_up_button_on(monitor->outside_panels[i]));
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
        }
        
        if (monitor->movementState == DOWN) {
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if (outsidePanel_is_down_button_on(monitor->outside_panels[i]) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if (outsidePanel_is_up_button_on(monitor->outside_panels[i]) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if (outsidePanel_is_down_button_on(monitor->outside_panels[i]) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
        }
        
        printf("OH NOS, I HAS NO FLOORS TO GO :/\n");
        //Espera por um andar
        pthread_cond_wait(&monitor->hasFloorToGo, &monitor->monitorGlobalLock);
        printf("Got signaled!\n");
    
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
    monitor->movementState = dir;
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    
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

void person_travel(ElevatorMonitor* monitor, int person_current_floor, int destiny){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    printf("person -> U la la, I has to go to floor %d\n", destiny);
    
    int inside = 0;
    //printf("person -> %d\n", monitor->currentFloor == person_current_floor && monitor->doorState == DOOR_OPEN);
    if(monitor->currentFloor == person_current_floor && monitor->doorState == DOOR_OPEN){
        //printf("Pressed the button for my floor\n");
        inside = 1;
    }

    int dir = UP;
    
    while(!inside) {
        //TODO: Transformar em while
        if(person_current_floor < destiny){
            if (!outsidePanel_is_up_button_on(monitor->outside_panels[person_current_floor])) {
                outsidePanel_turn_on_up_button(monitor->outside_panels[person_current_floor]);
                pthread_cond_signal(&monitor->hasFloorToGo);
            }
            dir = UP;
            printf("Button up -> %d\n", outsidePanel_is_up_button_on(monitor->outside_panels[person_current_floor]));
            pthread_cond_wait(monitor->floorUpConditions+person_current_floor, &monitor->monitorGlobalLock);
            
        }
        else{
            if (!outsidePanel_is_down_button_on(monitor->outside_panels[person_current_floor])) {
                outsidePanel_turn_on_down_button(monitor->outside_panels[person_current_floor]);
                pthread_cond_signal(&monitor->hasFloorToGo);
            }
            dir = DOWN;
            
            pthread_cond_wait(monitor->floorDownConditions+person_current_floor, &monitor->monitorGlobalLock);
            
        }
        //Pode entrar?
        if(monitor->people_inside < monitor->capacity){
            inside = 1;
            
            insidePanel_press_button(monitor->inside_panel, destiny);
            pthread_cond_signal(&monitor->hasFloorToGo);
            
            //Avisa para mais pessoas entrarem
            if(dir == UP){
                pthread_cond_signal(monitor->floorUpConditions+person_current_floor);
            }
            else{
                pthread_cond_signal(monitor->floorDownConditions+person_current_floor);
            }
        }
    }
    
    //Entra
    monitor->people_inside++;
    printf("Entrei!\n");
    //Aperta o botao para o andar de destino
    insidePanel_press_button(monitor->inside_panel, destiny);
    printf("ta apertado %d ? %d\n", destiny, insidePanel_is_button_on(monitor->inside_panel, destiny));
    pthread_cond_signal(&monitor->hasFloorToGo);
    printf("Sinalizei!\n");
    
    printf("Esperando\n");
    //Espera chegar no andar
    pthread_cond_wait(monitor->floorLeaveConditions+destiny, &monitor->monitorGlobalLock);
    
    monitor->people_inside--;
    
    //Sai, avisa mais alguem pra sair
    pthread_cond_signal(monitor->floorLeaveConditions+destiny);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    return;
}


void person_visit(int miliseconds){
    
    usleep(miliseconds*1000);
    
}



void person_end(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    monitor->number_of_clients--;
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}

