//
//  main.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "monitorElevatorSide.h"
#include "monitorUserSide.h"

pthread_t tid[2];

void* elevator(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    int next_floor;
    int current_floor;
    while (!elevator_should_end(monitor)) {
        next_floor = elevator_get_next_floor(monitor);
        printf("Next Floor -> %d\n", next_floor);
        current_floor = elevator_get_current_floor(monitor);
        if (next_floor == current_floor) {
            elevator_open_doors(monitor);
            elevator_wait_on_floor(monitor);
            elevator_close_doors(monitor);
//            elevator_move(monitor, elevator_get_current_movement_state(monitor));
        }else{
            if (next_floor > current_floor) {
                elevator_move(monitor, UP);
            }
            else{
                elevator_move(monitor, DOWN);
            }
            
        }
    }
    
    printf("Elevator end\n");
    return 0;
}

void* person(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    
    int number_of_floor_to_visit = 2;
    
    int floors[2] = {1, 3};
    int visit_time[2] = {3, 5};
    int current_floor = 0;
    
    for (int i = 0; i < number_of_floor_to_visit; i++) {
        person_travel(monitor, current_floor, floors[i]);
        printf("Visitando %d\n", floors[i]);
        current_floor = floors[i];
        person_visit(visit_time[i]);
    }
    
    
    person_end(monitor);
    
    
    
    return NULL;
}


int main(void)
{
//    int i = 0;
//    int err;
    
    printf("Starting \n");
    ElevatorMonitor* monitor = new_elevator_monitor(3, 5, 1);
    
    pthread_t  elevador, pessoa;
    pthread_create(&elevador, NULL, elevator, (void*)monitor);
    pthread_create(&pessoa, NULL, person, (void*)monitor);
//    while(i < 2)
//    {
//        err = pthread_create(&(tid[i]), NULL, elevator, NULL);
//        if (err != 0)
//            printf("\ncan't create thread :[%s]", strerror(err));
//        else
//            printf("\n Thread created successfully\n");
//        
//        i++;
//    }
    pthread_join(elevador, NULL);
    pthread_join(pessoa, NULL);
    sleep(5);
    return 0;
}