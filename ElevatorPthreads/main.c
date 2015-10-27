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



struct clientArgs{
    ElevatorMonitor *monitor;
    int id;
    int floor[50];
    int tempo_visita[50];
    int itinerary_size;
};
typedef struct clientArgs client_args;


//Algoritmo do elevador
void* elevator(void *arg)
{
    ElevatorMonitor* monitor = (ElevatorMonitor*)arg;
    int next_floor;
    int current_floor;
    int should_change_direction = 0; //Contador auxiliar
    
    //elevator_wait_on_floor(monitor);
    
    //Roda até o programa terminar
    while (1){
        
        //Descobre andar atual e o próximo a ser visitado
        next_floor = elevator_get_next_floor(monitor);
        current_floor = elevator_get_current_floor(monitor);
        
        //Se o proximo andar a parar for igual ao atual
        if (next_floor == current_floor && should_change_direction < 2) {
            
            //Se o contador é um, é pq, sem se mover, ja teve que parar duas vezes no mesmo andar, logo, deve mudar de sentido
            if (should_change_direction == 1) {
                
                if (elevator_get_current_movement_state(monitor) == UP) {
                    elevator_set_current_movement_state(monitor, DOWN);
                }
                else{
                    
                    elevator_set_current_movement_state(monitor, UP);
                    
                }
                //Senao abre a porta e espera no andar
            }else{
                elevator_open_doors(monitor,NULL);
                elevator_wait_on_floor(monitor);
            }

            should_change_direction++;
            
            //Caso seja diferente, sobe ou desce dependo do proximo andar
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
        //Se o contador é dois, já parou para subirem e descerem no andar. Move, independente de quem esteja esperando
        else{
            should_change_direction = 0;
            elevator_close_doors(monitor, NULL);
            elevator_move(monitor, elevator_get_current_movement_state(monitor));
        }
    }
    return 0;
}



//Código executado por um cliente
void* person(void *arg)
{
    
    client_args *args = (client_args*)arg;
    ElevatorMonitor* monitor = args->monitor;
    int current_floor = 0;
    
    
    for (int i = 0; i < args->itinerary_size; i++) {
        //Viaja
        person_travel(monitor, args->id, current_floor, args->floor[i], NULL);
        current_floor = args->floor[i];
        //Visita
        person_visit(args->tempo_visita[i]);
    }
    
    person_end(monitor, args->id, NULL);
    
    return NULL;
}



int main(void)
{
    int number_of_clients;
    pthread_t clients[100];

    scanf("%d", &number_of_clients);
    ElevatorMonitor* monitor = new_elevator_monitor(3, 5, 1);
    
    client_args *args[100];
    
    //Inicia as threads
    for(int i = 0; i < number_of_clients ; i ++){
        printf("%d 0 E 0\n", i+1);
    }
    
    pthread_t elevador;
    
    //Cria a thread do elevador
    pthread_create(&elevador, NULL, elevator, (void*)monitor);
    
    
    
    //Cria os clientes com seus respectivos atributos
    for(int i = 0 ; i < number_of_clients ; i++){
        args[i] = (client_args*) malloc(sizeof(client_args));
        args[i]->id = i+1;
        args[i]->monitor = monitor;
        scanf("%d\n", &args[i]->itinerary_size);
        for (int j = 0; j < args[i]->itinerary_size; j++) {
            scanf("%d", args[i]->floor + j);
            scanf("%d", args[i]->tempo_visita + j);
        }
        //Adiciona uma rota para retornar para o térreo
        args[i]->floor[args[i]->itinerary_size] = 0;
        args[i]->tempo_visita[args[i]->itinerary_size] = 0;
        args[i]->itinerary_size++;

    }
    
    //Inicia os clientes
    for (int i = 0; i < number_of_clients; i++) {
        pthread_create(clients + i, NULL, person, args[i]);
    }
    
    //Espera os clientes morrerem e libera seus argumentos
    for (int i = 0; i < number_of_clients; i++) {
        pthread_join(clients[i], NULL);
        free(args[i]);
    }
    
    //Print final
    printf("0 0 0 0\n");
}