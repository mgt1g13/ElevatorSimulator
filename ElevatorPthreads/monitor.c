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
#include <sys/timeb.h>
#include <unistd.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

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
    pthread_mutex_t monitorGlobalLock; //Lock do monitor
    pthread_cond_t timedWait; //Variável de condição para esperar com tempo limitado
    pthread_cond_t *floorUpConditions; //Variáveis de condição para os clientes que querem subir
    pthread_cond_t *floorDownConditions; // Variáveis de condição para os clientes que querem descer
    pthread_cond_t *floorLeaveConditions; //Variáveis de condição para os clientes que estão no elevador esperando para descer
    pthread_cond_t hasFloorToGo; //Variável de condição em que elevador espera caso não possua andares para se mover
    
    //Painéis
    insidePanel *inside_panel;
    outsidePanel **outside_panels;
    
    int number_of_clients;
    int doorState;
    int currentFloor;
    direction movementState;
    int capacity;
    int numberOfFloors;
    int people_inside;
    int* destinies; //Vetor do tamanho do número de andares. Um contador para cada andar com o número de pessoas que quer descer no mesmo

    struct timespec start_time; //Começo da execução
    
};


//Construtor
ElevatorMonitor* new_elevator_monitor(int capacity, int numberOfFloors, int number_of_clients){
    
    ElevatorMonitor *new_monitor = (ElevatorMonitor*)malloc(sizeof(ElevatorMonitor));
   
    new_monitor->numberOfFloors = numberOfFloors;
    new_monitor->capacity = capacity;
    new_monitor->doorState  = DOOR_CLOSED; //Inicia com a porta fechada
    new_monitor->currentFloor = TERREO; //No térreo
    new_monitor->people_inside = 0;
    new_monitor->movementState = UP; //Inicia subindo
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
    
    
    //Aloca painéis
    new_monitor->inside_panel = new_insidePanel(numberOfFloors);
    
    new_monitor->outside_panels = (outsidePanel**)malloc(numberOfFloors*sizeof(outsidePanel*));
    for (int i = 0; i < numberOfFloors ; i++) {
        new_monitor->outside_panels[i] = new_outsidePanel();
    }
    
    //Pega a data da criação do monitor
    
#ifdef __MACH__ // OS X
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    new_monitor->start_time.tv_sec = mts.tv_sec;
    new_monitor->start_time.tv_nsec = mts.tv_nsec;
    
#else
    clock_gettime(CLOCK_REALTIME, &new_monitor->start_time);
#endif
    
    return new_monitor;
}




//Fecha a porta
void elevator_close_doors(ElevatorMonitor* monitor, buffer* buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);

    
    //Verifica se todo mundo que tinha que sair já saiu
    while (monitor->destinies[monitor->currentFloor] != 0) {
        pthread_mutex_unlock(&monitor->monitorGlobalLock);

        //Senão saíram, espera mais um pouco
        elevator_wait_on_floor(monitor);
        
        pthread_mutex_lock(&monitor->monitorGlobalLock);
    }

    //Fecha a porta
    if(monitor->doorState != DOOR_CLOSED){
        buffer_write(buff, 0,  monitor->start_time, 'F', monitor->currentFloor);
        monitor->doorState = DOOR_CLOSED;
    }
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}



//Espera sobre a condicao do timedWait
void _wait(ElevatorMonitor* monitor, int nseconds){
    
    struct timespec ts;
    struct timeval tp;
    
    //Set wait time
    gettimeofday(&tp, NULL);
    
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    //ts.tv_sec += seconds;
    ts.tv_nsec += nseconds ;
    pthread_cond_timedwait(&monitor->timedWait, &monitor->monitorGlobalLock, &ts);
    
}



//Para no andar e espera as pessoas entrarem/sairem
void elevator_wait_on_floor(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
//    buffer_write(NULL, 0, monitor->start_time, 'B', monitor->currentFloor);
    _wait(monitor, PEOPLE_ENTER_TIME);
//    buffer_write(NULL, 0, monitor->start_time, 'E', monitor->currentFloor);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}

//Abre as portas
void elevator_open_doors(ElevatorMonitor* monitor, buffer* buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    //Se não está aberta, abre. Pela lógica, abrir a porta chama pessoas para entrar
    //Se ele vai inverter de sentido em um andar, essa função pode ser chamada duas
    //vezes
    
    if (monitor->doorState != DOOR_OPEN) {
        buffer_write(buff, 0, monitor->start_time, 'A', monitor->currentFloor);
        monitor->doorState = DOOR_OPEN;
    }
    
    
    //Sinaliza clientes esperando no andar
    pthread_cond_signal(monitor->floorLeaveConditions + monitor->currentFloor);
    
    //Desliga o botão interno se ele não estiver desligado
    if (insidePanel_is_button_on(monitor->inside_panel, monitor->currentFloor)) {
        buffer_write(buff, 0, monitor->start_time, 'i', monitor->currentFloor);
        
        insidePanel_turn_off_button(monitor->inside_panel, monitor->currentFloor);

    }
    
    //Espera pessoas saírem
    _wait(monitor, TIME_FOR_PEOPLE_TO_LEAVE);
    
    
    //Desbloqueia as pessoas que querem subir
    if(monitor->movementState == UP){
        
        //Desliga o botao de subir do painel externo se estiver ligado
        if (outsidePanel_is_up_button_on(monitor->outside_panels[monitor->currentFloor])) {
            buffer_write(buff, 0, monitor->start_time, 's', monitor->currentFloor);
            outsidePanel_turn_off_up_button(monitor->outside_panels[monitor->currentFloor]);

        }

        //Avisa quem quiser subir
        pthread_cond_signal(monitor->floorUpConditions + monitor->currentFloor);
        
    }
    
    //Acorda quem quer descer
    if(monitor->movementState == DOWN){

        //Desliga o botão de descer do painel externo
        if (outsidePanel_is_down_button_on(monitor->outside_panels[monitor->currentFloor])) {
            buffer_write(buff, 0, monitor->start_time, 'd', monitor->currentFloor);
            outsidePanel_turn_off_down_button(monitor->outside_panels[monitor->currentFloor]);
            
        }
        pthread_cond_signal(monitor->floorDownConditions + monitor->currentFloor);
    }
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}


//Descobre o próximo andar para se mover
int elevator_get_next_floor(ElevatorMonitor *monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);

    //Procura o próximo andar para qual o elevador deve se mover
    while (1) {
        
        int full = monitor->people_inside == monitor->capacity;
        
        //Caso esteja subindo
        if (monitor->movementState == UP) {
            
            //Próximo andar acima em que alguém queira sair ou que alguém esteja esperando para subir
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if ( (outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full)  || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            //Andar mais acima com alguém que queira descer
            for (int i = monitor->numberOfFloors-1; i > monitor->currentFloor; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i])&& !full)){// || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            // O andar mais próximo abaixo com clientes querendo descer
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            // O andar mais abaixo com clientes querendo subir
            for (int i = 0; i < monitor->currentFloor; i++) {
                if ((outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full)){// || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }

        }
        
        if (monitor->movementState == DOWN) {
            
            //Primeiro andar abaixo em que alguém vá sair ou em que alguém queira descer
            for (int i = monitor->currentFloor; i >= 0; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            //Andar mais abaixo em que haja um cliente querendo subir
            for (int i = 0; i < monitor->currentFloor; i++) {
                if ((outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            //Próximo andar acima em que haja algum cliente querendo subir
            for (int i = monitor->currentFloor; i < monitor->numberOfFloors; i++) {
                if ((outsidePanel_is_up_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }
            
            //Andar mais acima em que haja algum cliente querendo descer
            for (int i =  monitor->numberOfFloors-1; i >monitor->currentFloor; i--) {
                if ((outsidePanel_is_down_button_on(monitor->outside_panels[i]) && !full) || insidePanel_is_button_on(monitor->inside_panel, i)) {
                    pthread_mutex_unlock(&monitor->monitorGlobalLock);
                    return i;
                }
            }

            
        }
        
        
        
        //Espera ser chamado
        pthread_cond_wait(&monitor->hasFloorToGo, &monitor->monitorGlobalLock);
    }

}

//Retorna andar atual
int elevator_get_current_floor(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    int ret = monitor->currentFloor;
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    
    return ret;
}

//Retorna sentido do movimento
direction elevator_get_current_movement_state(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    direction ret = monitor->movementState;
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    
    return ret;
    
}

//Muda sentido do movimento
void elevator_set_current_movement_state(ElevatorMonitor* monitor, direction dir, buffer *buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    if (monitor->doorState == DOOR_CLOSED) {
        printf("This function should only be called with open doors");
        exit(1);
    }
    
    monitor->movementState = dir;
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    elevator_open_doors(monitor, buff);
    elevator_wait_on_floor(monitor);
    
}

//Move o elevador. Deve ser protegida por exclusão mútua
void _move(ElevatorMonitor* monitor, direction dir){
    
    struct timespec ts;
    struct timeval tp;
    
    //Set wait time
    gettimeofday(&tp, NULL);
    
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += SECONDS_BETWEEN_FLOORS;
    
    pthread_cond_timedwait(&monitor->timedWait, &monitor->monitorGlobalLock, &ts);

    monitor->currentFloor += dir;
}

//Move entra andares com o sentido dado por dir
void _elevator_move_between_floors(ElevatorMonitor* monitor, direction dir){
    _move(monitor, dir);
    monitor->movementState = dir;
}



//Interface de movimento
void elevator_move(ElevatorMonitor* monitor, direction dir){
    pthread_mutex_lock(&monitor->monitorGlobalLock);

    //Inverte o sentido se o elvador for sair dos seus limites
    if (monitor->currentFloor == monitor->numberOfFloors - 1) {
        dir = DOWN;
    }
    else if (monitor->currentFloor == 0){
        dir = UP;
    }
    

    _elevator_move_between_floors(monitor, dir);
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}

//DEPRECATED
int elevator_should_end(ElevatorMonitor* monitor){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    
    int should_end = monitor->number_of_clients?0:1;
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
    return should_end;
}

//Pessoa viaja
void person_travel(ElevatorMonitor* monitor, int thread, int person_current_floor, int destiny, buffer* buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    
    int inside = 0;
    
    //Se a pessoa tem que ir para o mesmo andar que já está, retorna
    if (person_current_floor == destiny) {
        pthread_mutex_unlock(&monitor->monitorGlobalLock);
        return;
    }

    
    //Descobre a direção do movimento da pessoa
    int dir = UP;
    if (destiny < person_current_floor) {
        dir = DOWN;
    }
    else{
        dir = UP;
    }
    
    //Se o elevador esta parado, com a porta aberta, indo para o mesmo sentido e não lotado, entra
    if(monitor->currentFloor == person_current_floor
       && monitor->doorState == DOOR_OPEN
       && monitor->people_inside < monitor->capacity
       && monitor->movementState == dir){
        inside = 1;
    }


    //Enquanto não entra, tenta entrar
    while(!inside) {
        
        //Se quer subir
        if(person_current_floor < destiny){

            //Aperta o botão no painel
            if (!outsidePanel_is_up_button_on(monitor->outside_panels[person_current_floor])) {
                buffer_write(buff, thread, monitor->start_time, 'S', person_current_floor);
                outsidePanel_turn_on_up_button(monitor->outside_panels[person_current_floor]);

            }
            
            //Sinaliza para o elevador que ele tem um andar para ir.
            pthread_cond_signal(&monitor->hasFloorToGo);
            dir = UP;
            
            
            buffer_write(buff, thread, monitor->start_time, 'B', monitor->currentFloor);

            //Espera o elevador chegar e chamar as pessoas que querem subir
            pthread_cond_wait(monitor->floorUpConditions+person_current_floor, &monitor->monitorGlobalLock);
            
            buffer_write(buff, thread, monitor->start_time, 'E', monitor->currentFloor);
            
            
        }
        //Se quer descer
        else if(person_current_floor > destiny){
            //Aperta o botão
            if (!outsidePanel_is_down_button_on(monitor->outside_panels[person_current_floor])) {
                buffer_write(buff, thread, monitor->start_time, 'D', person_current_floor);
                outsidePanel_turn_on_down_button(monitor->outside_panels[person_current_floor]);

            }
            pthread_cond_signal(&monitor->hasFloorToGo);
            dir = DOWN;
            
            buffer_write(buff, thread, monitor->start_time, 'B', monitor->currentFloor);

            //Espera o elevador chamar e verificar se pode descer
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
        //Se não pode entrar, espera um tempo até chamar de novo
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

//Visita
void person_visit(int miliseconds){
    //Dorme por uma quantia de nano segundos
    usleep(miliseconds/1000);
    
}


//Pessoa termina - Morre
void person_end(ElevatorMonitor* monitor, int thread, buffer *buff){
    pthread_mutex_lock(&monitor->monitorGlobalLock);
    

    monitor->number_of_clients--;
    buffer_write(buff, thread, monitor->start_time, 'M', 0);
    
    
    pthread_mutex_unlock(&monitor->monitorGlobalLock);
}

