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
#include "buffer.h"


pthread_t tid[2];



struct elevatorArgs{
    ElevatorMonitor *monitor;
    buffer *buff;
    
};
typedef struct elevatorArgs elevator_args;



struct clientArgs{
    ElevatorMonitor *monitor;
    int id;
    int floor[50];
    int tempo_visita[50];
    int itinerary_size;
    buffer *buff;
    
};
typedef struct clientArgs client_args;




//Algoritmo do elevador
void* elevator(void *arg)
{
    elevator_args* elArg = (elevator_args*) arg;
    ElevatorMonitor* monitor = elArg->monitor;
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
                    elevator_set_current_movement_state(monitor, DOWN, elArg->buff);
                }
                else{
                    
                    elevator_set_current_movement_state(monitor, UP, elArg->buff);
                    
                }
                //Senao abre a porta e espera no andar
            }else{
                elevator_open_doors(monitor,elArg->buff);
                elevator_wait_on_floor(monitor);
            }

            should_change_direction++;
            
            //Caso seja diferente, sobe ou desce dependo do proximo andar
        }else if(next_floor != current_floor){
            elevator_close_doors(monitor, elArg->buff);
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
            elevator_close_doors(monitor, elArg->buff);
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
        person_travel(monitor, args->id, current_floor, args->floor[i], args->buff);
        current_floor = args->floor[i];
        //Visita
        person_visit(args->tempo_visita[i]);
    }
    
    person_end(monitor, args->id, args->buff);
    
    return NULL;
}



int compara(const void* arg1, const void *arg2){
    op* op1 = (op*)arg1;
    op* op2 = (op*)arg2;
//    printf("lala\n");
    
    if (op1->timestamp == op2->timestamp) {
        //Prioridade pra quem abre a porta
        if (op1->op == 'A') {
            return -1;
        }
        else if (op2->op == 'A'){
            return 1;
        }
        else{
            return 0;
        }
    }
    else if(op1->timestamp < op2->timestamp){
        return -1;
    }
    else
        return 1;
    
    
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
    
    
    struct elevatorArgs elArg;
    
    
    elArg.monitor = monitor;
    elArg.buff = new_buffer(300000);
    
    //Cria a thread do elevador
    pthread_create(&elevador, NULL, elevator, (void*)&elArg);
    
    
    
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
        args[i]->buff = new_buffer(700);

    }
    
    //Inicia os clientes
    for (int i = 0; i < number_of_clients; i++) {
        pthread_create(clients + i, NULL, person, args[i]);
    }
    
    op *operations = (op*)malloc(90000*sizeof(op));
    //op operations[300000];
    int cont = 0;
    
    //Espera os clientes morrerem e libera seus argumentos
    for (int i = 0; i < number_of_clients; i++) {
        pthread_join(clients[i], NULL);
        for(int j = 0; j < args[i]->buff->n_ops ; j++){
            operations[cont].thread = args[i]->buff->ops[j].thread;
           // printf("%lli\n", args[i]->buff->ops[j].timestamp);
            operations[cont].timestamp = args[i]->buff->ops[j].timestamp;
            operations[cont].op = args[i]->buff->ops[j].op;
            operations[cont].floor = args[i]->buff->ops[j].floor;
            cont++;
        }
        free(args[i]);
        
    }
    for (int i = 0; i < elArg.buff->n_ops; i++) {
        operations[cont].thread = elArg.buff->ops[i].thread;
        operations[cont].timestamp = elArg.buff->ops[i].timestamp;
        operations[cont].op = elArg.buff->ops[i].op;
        operations[cont].floor = elArg.buff->ops[i].floor;
        
//        printf("->%c\n", elArg.buff->ops[i].op);
        cont++;
    }
    
    qsort(operations, cont, sizeof(op), compara);

    
    for (int i = 0; i < cont; i++) {
        printf("%d %lli %c %d\n", operations[i].thread, operations[i].timestamp, operations[i].op, operations[i].floor);
    }

    //Print final
    printf("0 0 0 0\n");
}