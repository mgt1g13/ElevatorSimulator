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

//Implementação de um painel interno, com seus botões


struct insidePanel{
    int* buttons;
    int numberOfFloor;
};

insidePanel* new_insidePanel(int numberOfFloors){
    insidePanel* new_panel = (insidePanel*)malloc(sizeof(insidePanel));
    new_panel->buttons = (int*)malloc((numberOfFloors)*sizeof(int));
    memset(new_panel->buttons, 0, (numberOfFloors)*sizeof(int));
    new_panel->numberOfFloor = numberOfFloors;
    
    
    return new_panel;
    
}

void insidePanel_press_button(insidePanel* panel, int floor){
    panel->buttons[floor] = 1;
}

int insidePanel_is_button_on(insidePanel* panel, int floor){
    return panel->buttons[floor];
    
}


void insidePanel_turn_off_button(insidePanel* panel, int floor){
    panel->buttons[floor] = 0;
}

void insidePanel_destructor(insidePanel* panel){
    free(panel->buttons);
    free(panel);
}

