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

//Interface do painel externo

typedef struct outsidePanel outsidePanel;


//Construtor
outsidePanel* new_outsidePanel();

//Liga botão de subir
void outsidePanel_turn_on_up_button(outsidePanel* panel);

//Desliga botão de subir
void outsidePanel_turn_off_up_button(outsidePanel* panel);

//Verifica se o botão de subir está ligado
int outsidePanel_is_up_button_on(outsidePanel *panel);

//Liga botão de descer
void outsidePanel_turn_on_down_button(outsidePanel* panel);

//Desliga botão de descer
void outsidePanel_turn_off_down_button(outsidePanel* panel);

//Verifica se o botão de descer está ligado
int outsidePanel_is_down_button_on(outsidePanel *panel);

//Libera a memoria do painel
void outsidePanel_destructor(outsidePanel* panel);

#endif /* defined(__ElevatorPthreads__outsidePanel__) */
