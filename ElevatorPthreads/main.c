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
//char ***buffer;

void* elevator(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    int next_floor;
    int current_floor;
    int should_change_direction = 0;
    char **buffer;
    
    elevator_wait_on_floor(monitor);
    
//    buffer = (char**)malloc(400*sizeof(char*));
//    for(int i = 0 ; i < 400 ; i++){
//        buffer[i] = (char*)malloc(100*sizeof(char));
//    }
    
    
    //int buffer_position = 0;
    
    while (1){
        
        next_floor = elevator_get_next_floor(monitor);
      //  printf("Next Floor -> %d\n", next_floor);
        current_floor = elevator_get_current_floor(monitor);

        if (next_floor == current_floor && should_change_direction < 2) {
            elevator_open_doors(monitor,NULL);
            elevator_wait_on_floor(monitor);
            if(should_change_direction == 0){
        //        printf("Incrementing should change direction\n");
                should_change_direction++;
            }
            else{
                should_change_direction = 0;
                if (elevator_get_current_movement_state(monitor) == UP) {
                    elevator_set_current_movement_state(monitor, DOWN);
                }
                else{
                    elevator_set_current_movement_state(monitor, UP);
                }
            }

        }else if(should_change_direction < 2){
            elevator_close_doors(monitor, NULL);
            should_change_direction = 0;
            if (next_floor > current_floor) {
                elevator_move(monitor, UP);
            }
            else{
                elevator_move(monitor, DOWN);
            }
            
        }
        else{
            elevator_close_doors(monitor, NULL);
            elevator_move(monitor, elevator_get_current_movement_state(monitor));
        }
    }
    return 0;
}

void* person(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    
    int number_of_floor_to_visit = 2;
    
    int floors[2] = {1, 3};
    int visit_time[2] = {6, 0};
    int current_floor = 0;
    
    for (int i = 0; i < number_of_floor_to_visit; i++) {
        person_travel(monitor, 1 ,current_floor, floors[i], NULL);
        //printf("Visitando %d\n", floors[i]);
        current_floor = floors[i];
        person_visit(visit_time[i]);
    }
    
    
    person_end(monitor);
    
    
    
    return NULL;
}

void* person2(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    
    int number_of_floor_to_visit = 2;
    
    int floors[2] = {2, 4};
    int visit_time[2] = {0, 0};
    int current_floor = 0;
    
    for (int i = 0; i < number_of_floor_to_visit; i++) {
        person_travel(monitor, 2,current_floor, floors[i], NULL);
        //printf("Visitando %d\n", floors[i]);
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
    
    //printf("Starting \n");
    ElevatorMonitor* monitor = new_elevator_monitor(3, 5, 1);
    
    printf("1 0 E 0\n");
    printf("2 0 E 0\n");
    printf("0 0 A 0\n");
    
    
    pthread_t  elevador, pessoa, pessoa2;
    pthread_create(&elevador, NULL, elevator, (void*)monitor);
    pthread_create(&pessoa, NULL, person, (void*)monitor);
    pthread_create(&pessoa2, NULL, person2, (void*)monitor);

    
    pthread_join(pessoa, NULL);
    pthread_join(pessoa2, NULL);
    printf("0 0 0 0\n");
    //sleep(5);
    return 0;
}