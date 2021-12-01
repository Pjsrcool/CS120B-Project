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
#include "../header/io.h"
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

unsigned char isP1LeftFoot() {
    unsigned long value =  ADC;

    if (value <= 48)
        return 1;
    else
        return 0;
}

unsigned char isP1RightFoot() {
    unsigned long value = ADC;

    if (value >= 975)
        return 1;
    else
        return 0;
}

unsigned char update = 1;
unsigned char player1 = 0;
unsigned char player1finish = 0;

enum Start {Wait, Press, Release};
int StartButton(int state) {
    unsigned char button = PINB & 0x08;
    switch(state) {
        case Wait: 
            if (button == 0x00)
                state = Press;
            else
                state = Wait;
            break;
        case Press:
            if (button == 0x08)
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
            if (player1 == 0) {
                player1 = 1;
            } else {
                player1 = 0;
            }
            update = 1;
            break;
        default: break;
    }

    return state;
}


unsigned short P1currentDistance = 0;
const unsigned short raceDistance = 40;
const unsigned char LED[41] = {2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,};
const unsigned char LeftSteps[41] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
const unsigned char RightSteps[41]= {0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0};

enum StepGamePlayer1 {Off, go, finish };
int StepGamePlayer1(int state) {
    switch (state) {
        case Off : 
            if (player1 == 0) 
                state = Off;
            else {
                P1currentDistance = 0;
                player1finish = 0;
                state = go;
            }
            break;
        case go :
            if (player1 == 0)
                state = Off; 
            else if (P1currentDistance >= raceDistance)
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
            PORTB = 0;
        case go : 
            if (P1currentDistance < raceDistance) {
                PORTB = LED[P1currentDistance] & 0x03;
                if (isP1LeftFoot() == LeftSteps[P1currentDistance] && isP1RightFoot() == RightSteps[P1currentDistance])
                    P1currentDistance++;
            }
            break;
        case finish:
            player1finish = 1;
            player1 = 0;
            update = 1;
            break;
    }
    return state;
}

unsigned char player2finish = 0;

enum StepGamePlayer2 {getData};
int StepGamePlayer2(int state) {
    static unsigned char pC;
    unsigned char old = pC;
    pC = PINC & 0x01;

    switch(state) {
        case getData:
            state = getData;
            player2finish = pC;
            if (player2finish != old)
                update = 1;
            break;
        default:
            state = getData;
            break;
    }
    return state;
}

// writes Game text to the LCD Display
enum LCDTextStates { wait, updateDisplay, gameOver};
int LCDTextTick(int state) {
    static int pointsP1 = 0, pointsP2 = 0;

    switch (state) {
        case wait:
            if (update)
                state = updateDisplay;
            else
                state = wait;
            break;
        case updateDisplay:
            if (pointsP1 >= 3 || pointsP2 >= 3)
                state = gameOver;
            else
                state = wait;
            break;
        case gameOver: break;
        default: state = wait; break;
    }

    switch (state) {
        case wait:
            break;
        case updateDisplay:
            if (player1finish && player2finish)
                LCD_DisplayString(1, "Both players    finished");
            else if (player1finish) {
                LCD_DisplayString(1, "P1 finished     first! +1 pt!");
                pointsP1++;
            } else if (player2finish) {
                LCD_DisplayString(1, "P2 finished     first! +1 pt!");
                pointsP2++;
            } else if (player1 == 1 && !player1finish && !player2finish) {
                LCD_DisplayString(1, "Goooooo");
            } else if (player1 == 0)
                LCD_DisplayString(1, "Push Button to  start.");
            
            update = 0;
            break;
        case gameOver:
            if (pointsP1 > pointsP2)
                LCD_DisplayString(1, "P1 wins! Reset  power please");
    }

    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; // PORTA = 0x00;
    DDRB = 0xF7; PORTB = 0x08;
    DDRC = 0x00;
    DDRD = 0xff;

    unsigned char period = 50;
    /*
    * TASKS
    */
    int numTasks = 4;
    int i = 0;
    task tasks [numTasks];

    // Start Button Task
    tasks[i].state = -1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StartButton;
    i++;

    // player1 task
    tasks[i].state = -1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StepGamePlayer1;
    i++;

    // player2 task
    tasks[i].state = -1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StepGamePlayer2;
    i++;

    // LCD Text
    tasks[i].state = -1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &LCDTextTick;
    i++;
    // Output

    LCD_init();
    LCD_DisplayString(1, "initializing");

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
