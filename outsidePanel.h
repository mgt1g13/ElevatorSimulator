//
//  outsidePanel.h
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#ifndef __ElevatorPthreads__outsidePanel__
#define __ElevatorPthreads__outsidePanel__

#include <stdio.h>

typedef struct outsidePanel outsidePanel;



void outsidePanel_turn_on_up_button(outsidePanel* panel);
void outsidePanel_turn_off_up_button(outsidePanel* panel);

void outsidePanel_turn_on_down_button(outsidePanel* panel);
void outsidePanel_turn_off_down_button(outsidePanel* panel);

#endif /* defined(__ElevatorPthreads__outsidePanel__) */
