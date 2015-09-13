//
//  insidePanel.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//

#include "insidePanel.h"
#include <stdlib.h>
#include <string.h>

#define BUTTON(x) x-1


struct insidePanel{
    
    int* buttons;
    
};

insidePanel* new_insidePanel(int numberOfFloors){
    insidePanel* new_panel = (insidePanel*)malloc(sizeof(insidePanel));
    new_panel->buttons = (int*)malloc(numberOfFloors*sizeof(int));
    memset(new_panel->buttons, 0, numberOfFloors*sizeof(int));
    
    return new_panel;
    
}



void insidePanel_press_button(insidePanel* panel, int floor){
    panel->buttons[BUTTON(floor)] = 1;
}


void insidePanel_turn_off_button(insidePanel* panel, int floor){
    
    
    
    
}

