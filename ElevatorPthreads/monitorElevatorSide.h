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

int elevator_should_end(ElevatorMonitor* monitor);
void elevator_move(ElevatorMonitor* monitor, direction dir);
int elevator_get_next_floor(ElevatorMonitor *monitor);
void elevator_close_doors(ElevatorMonitor* monitor);
void elevator_open_doors(ElevatorMonitor* monitor);
int elevator_get_current_floor(ElevatorMonitor* monitor);
direction elevator_get_current_movement_state(ElevatorMonitor* monitor);
void elevator_wait_on_floor(ElevatorMonitor* monitor);

#endif
