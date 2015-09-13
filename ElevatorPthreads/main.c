//
//  main.c
//  ElevatorPthreads
//
//  Created by Matheus Trindade on 13/09/15.
//  Copyright (c) 2015 Matheus Trindade. All rights reserved.
//
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

pthread_t tid[2];

void* doSomeThing(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();
    

    if(pthread_equal(id,tid[0]))
    {
        sleep(2);
        printf("\n First thread processing\n");
    }
    else
    {
        sleep(1);
        printf("\n Second thread processing\n");
    }
    
    //for(i=0; i<(0xFFFFFFFF);i++);
    
    return NULL;
}

int main(void)
{
    int i = 0;
    int err;
    
    while(i < 2)
    {
        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
        
        i++;
    }
    
    sleep(5);
    return 0;
}