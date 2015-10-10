//
//  monitorUserSide.h
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#ifndef ElevatorPthreads_monitorUserSide_h
#define ElevatorPthreads_monitorUserSide_h

#include "monitor.h"

void person_visit(int miliseconds);

void person_travel(ElevatorMonitor* monitor, int thread, int person_current_floor, int destiny, buffer* buff);

void person_end(ElevatorMonitor* monitor);


#endif
