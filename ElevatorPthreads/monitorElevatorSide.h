//
//  monitorElevatorSide.h
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#ifndef ElevatorPthreads_monitorElevatorSide_h
#define ElevatorPthreads_monitorElevatorSide_h

#include "monitor.h"
#include "buffer.h"

//Interface para o lado do elevador

// DEPRECATED
int elevator_should_end(ElevatorMonitor* monitor);

//Move o elevador para uma dada direção
void elevator_move(ElevatorMonitor* monitor, direction dir);

//Procura o próximo andar a visitar
int elevator_get_next_floor(ElevatorMonitor *monitor);

//Fecha as portas do elevador
void elevator_close_doors(ElevatorMonitor* monitor, buffer* buff);

//Abre as portas do elevador
void elevator_open_doors(ElevatorMonitor* monitor, buffer* buff);

//Descobre o andar atual
int elevator_get_current_floor(ElevatorMonitor* monitor);

//Descobre o sentido atual do movimento do elevador
direction elevator_get_current_movement_state(ElevatorMonitor* monitor);

//Espera no andar para entrarem e sairem pessoas
void elevator_wait_on_floor(ElevatorMonitor* monitor);

//Muda o sentido do movimento
void elevator_set_current_movement_state(ElevatorMonitor* monitor, direction dir);

#endif
