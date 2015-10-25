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

#define FILE_INPUT



pthread_t tid[2];



struct clientArgs{
    ElevatorMonitor *monitor;
    int id;
    int floor[50];
    int tempo_visita[50];
    int itinerary_size;
};
typedef struct clientArgs client_args;



void* elevator(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    int next_floor;
    int current_floor;
    int should_change_direction = 0;
    
    elevator_wait_on_floor(monitor);
    
    while (1){
        next_floor = elevator_get_next_floor(monitor);
        current_floor = elevator_get_current_floor(monitor);
        
        if (next_floor == current_floor && should_change_direction < 2) {

            if (should_change_direction == 1) {
                
                if (elevator_get_current_movement_state(monitor) == UP) {
                    elevator_set_current_movement_state(monitor, DOWN);
                }
                else{

                    elevator_set_current_movement_state(monitor, UP);
                    
                }
            }else{
                elevator_open_doors(monitor,NULL);
                elevator_wait_on_floor(monitor);
            }

            should_change_direction++;
            
        }else if(next_floor != current_floor){
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
            should_change_direction = 0;
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
    int visit_time[2] = {7, 0};
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

void* person3(void *arg)
{
    
    client_args *args = (client_args*)arg;
    ElevatorMonitor* monitor = args->monitor;
    int current_floor = 0;
    
    for (int i = 0; i < args->itinerary_size; i++) {
        person_travel(monitor, args->id, current_floor, args->floor[i], NULL);
        current_floor = args->floor[i];
        person_visit(args->tempo_visita[i]);
    }
    
    person_end(monitor);
    
    return NULL;
}



int main(void)
{
    
    
#ifndef FILE_INPUT
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

    
    return 0;
#else
    
    int number_of_clients;
    pthread_t clients[100];

    scanf("%d", &number_of_clients);
    ElevatorMonitor* monitor = new_elevator_monitor(3, 5, 1);
    
    client_args *args[100];
    
    for(int i = 0; i < number_of_clients ; i ++){
        printf("%d 0 E 0\n", i+1);
    }
    
    pthread_t elevador;
    
    pthread_create(&elevador, NULL, elevator, (void*)monitor);
    
    for(int i = 0 ; i < number_of_clients ; i++){
        args[i] = (client_args*) malloc(sizeof(client_args));
        args[i]->id = i+1;
        args[i]->monitor = monitor;
        scanf("%d\n", &args[i]->itinerary_size);
        for (int j = 0; j < args[i]->itinerary_size; j++) {
            scanf("%d", args[i]->floor + j);
            scanf("%d", args[i]->tempo_visita + j);
        }
        args[i]->floor[args[i]->itinerary_size] = 0;
        args[i]->tempo_visita[args[i]->itinerary_size] = 0;
        args[i]->itinerary_size++;
        
        //                pthread_create(clients + i, NULL, person3, args[i]);

    }
    
    for (int i = 0; i < number_of_clients; i++) {
        pthread_create(clients + i, NULL, person3, args[i]);
    }
    
    for (int i = 0; i < number_of_clients; i++) {
        pthread_join(clients[i], NULL);
    }
    
    for (int i = 0; i < number_of_clients; i++) {
        free(args[i]);
    }
#endif
}