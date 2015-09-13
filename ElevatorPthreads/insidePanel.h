//
//  insidePanel.h
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#ifndef __ElevatorPthreads__insidePanel__
#define __ElevatorPthreads__insidePanel__

#include <stdio.h>


typedef struct insidePanel insidePanel;

insidePanel* new_insidePanel(int numberOfSwitches);

void insidePanel_press_button(insidePanel* panel, int floor);

void insidePanel_turn_off_button(insidePanel* panel, int floor);


#endif /* defined(__ElevatorPthreads__insidePanel__) */
