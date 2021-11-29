/*	Author: justin
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 * 
 * Demo Link: https://drive.google.com/file/d/13dB-wkyHkuOn36P_5kCTMYPzrd4PjjQc/view?usp=sharing
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

enum Start {Wait, Press, Release};
int StartButton(int state) {
    unsigned char button = PINB & 0x01;

    switch(state) {
        case Wait: 
            if (button == 0x00)
                state = Press;
            else
                state = Wait;
            break;
        case Press:
            if (button == 0x01)
                state = Release;
            else
                state = Press;
            break;
        case Release:
            state = Wait;
            break;
        default: state = Wait; break;
    }

    switch(state) {
        case Wait: break;
        case Press: break;
        case Release: 
            if (player2 == 0) {
                // LCD_DisplayString(1,"GOOO");
                player2 = 1;
                // PORTD = 0x04;
            } else {
                // LCD_DisplayString(1,"Push Button to Start");
                player2 = 0;
                // PORTD = 0x00;
            }
            break;
        default: break;
    }

    return state;
}


unsigned short P2currentDistance = 0;
const unsigned short raceDistance = 40;
const unsigned char LeftSteps[41] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
const unsigned char RightSteps[41]= {0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0};

enum StepGameplayer2 {Off, go, finish };
int StepGameplayer2(int state) {
    // if (isP2RightFoot())
    // PORTD = 0xff;
    // else
    // PORTD = 0X00;
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
        case go : 
            if (P2currentDistance < raceDistance) {
                if (isP2LeftFoot() == LeftSteps[P2currentDistance] && isP2RightFoot() == RightSteps[P2currentDistance])
                    P2currentDistance++;
                // LCD_DisplayString(1,P2currentDistance + '0');
                // if (isP2LeftFoot()) {
                //     LCD_DisplayString(1, "left foot");
                //     currentDistance++;
                // } else if (isP2RightFoot()) {
                //     LCD_DisplayString(1, "right foot");
                //     currentDistance++;
                // } else {
                //     LCD_DisplayString(1, "waiting");
                // }
            }
            break;
        case finish:
            player2finish = 1;
            break;
        case Off: break;
    }
    // player2finish = 1;
    if (player2finish)
        PORTD = 0x01;
    else
        PORTD = 0X00;
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; // PORTA = 0x00;
    DDRB = 0x00; PORTB = 0x01;
    DDRD = 0xff; PORTD = 0x00;

    unsigned char period = 50;
    /*
    * TASKS
    */
    int numTasks = 2;
    int i = 0;
    task tasks [numTasks];

    // Start Button Task
    tasks[i].state = -1;
    tasks[i].period = 50;
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
