/*	Author: justin
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 * 
 * Demo Link: https://youtu.be/0gmFHRxrqv0
 */
#include <avr/io.h>
#include "../header/scheduler.h"
#include "../header/timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void ADC_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1<<ADATE);
    // ADEN: setting this bit enables analog-digital conversion
    // ADSC: setting this bit starts the first conversion
    // ADATE: setting this bit enables auto-triggering. 
    //        Since we are in Free Running mode, a new conversion 
    //        will trigger whenever the previous conversion completes
}

unsigned char isP2LeftFoot() {
    unsigned long value =  ADC;

    if (value <= 48)
        return 1;
    else
        return 0;
}

unsigned char isP2RightFoot() {
    unsigned long value = ADC;

    if (value >= 975)
        return 1;
    else
        return 0;
}

unsigned char player2 = 0;
unsigned char player2finish = 0;

enum Start {read};
int StartButton(int state) {
    unsigned char signal = PINB & 0x01;

    switch(state) {
        case read: 
            player2 = signal;
            state = read;
            break;
        default: 
            state = read;
            break;
    }

    return state;
}


unsigned short P2currentDistance = 0;
const unsigned short raceDistance = 40;
const unsigned char LED[41] = {4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2,4,4,2,2};
const unsigned char LeftSteps[41] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
const unsigned char RightSteps[41]= {0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0};


enum StepGameplayer2 {Off, go, finish };
int StepGameplayer2(int state) {
    unsigned char step, end;

    switch (state) {
        case Off : 
            if (player2 == 0) 
                state = Off;
            else {
                P2currentDistance = 0;
                player2finish = 0;
                state = go;
            }
            break;
        case go :
            if (player2 == 0)
                state = Off; 
            else if (P2currentDistance >= raceDistance)
                state = finish;
            else 
                state = go;
            break;
        case finish :
            state = Off;
            break;
        default : 
            state = Off; 
            break;
    }

    switch (state) {
        case Off: 
            step = 0;
            break;
        case go: 
            if (P2currentDistance < raceDistance) {
                step = LED[P2currentDistance] & 0x06;
                if (isP2LeftFoot() == LeftSteps[P2currentDistance] && isP2RightFoot() == RightSteps[P2currentDistance]){
                    P2currentDistance++;
                }
            }
            break;
        case finish:
            player2finish = 1;
            // player2 = 0;
            break;
    }

    PORTD = step | player2finish;
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; // PORTA = 0x00;
    DDRB = 0x00; PORTB = 0x00;
    DDRD = 0xff; PORTD = 0x00;

    unsigned char period = 25;
    /*
    * TASKS
    */
    int numTasks = 2;
    int i = 0;
    task tasks [numTasks];

    // Start Button Task
    tasks[i].state = -1;
    tasks[i].period = 25;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StartButton;
    i++;

    // player2 task
    tasks[i].state = -1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StepGameplayer2;
    i++;

    ADC_init();

    TimerSet(period);
    TimerOn();
    
    while (1) {
        // LCD_DisplayString(1, "inside loop");
        for(int j = 0; j < numTasks; ++j) {
            // LCD_DisplayString(1, "inside loop");
            if (tasks[j].elapsedTime >= tasks[j].period) {
                tasks[j].state = tasks[j].TickFct(tasks[j].state);
                tasks[j].elapsedTime = 0;
            }
            tasks[j].elapsedTime += period;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
