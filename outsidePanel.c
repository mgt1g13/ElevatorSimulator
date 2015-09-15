//
//  outsidePanel.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#include "outsidePanel.h"
#include <stdlib.h>

struct outsidePanel{
    int upButton;
    int downButton;
    
};


outsidePanel* new_outsidePanel(){
    
    outsidePanel* new_panel = (outsidePanel*)malloc(sizeof(outsidePanel));
    
    //Both buttons are initialized on of position
    new_panel->upButton = 0;
    new_panel->downButton = 0;
    
    return new_panel;
}


void outsidePanel_turn_on_up_button(outsidePanel* panel){
    panel->upButton = 1;
}

void outsidePanel_turn_off_up_button(outsidePanel* panel){
    panel->upButton = 0;
}


void outsidePanel_turn_on_down_button(outsidePanel* panel){
    panel->downButton = 1;
}

void outsidePanel_turn_off_down_button(outsidePanel* panel){
    panel->downButton = 0;
}

void outsidePanel_destructor(outsidePanel* panel){
    free(panel);
}