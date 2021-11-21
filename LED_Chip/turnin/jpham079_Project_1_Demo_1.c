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

unsigned char isLeftFoot() {
    unsigned char value = (unsigned char) ADC;

    return value <= 48;

    if (value <= 48)
        return 1;
    else
        return 0;
}

unsigned char isRightFoot() {
    unsigned char value = (unsigned char) ADC;

    return value >= 126;

    if (value >= 151)
        return 1;
    else
        return 0;
}

unsigned char player1 = 0;
unsigned char player1finish = 0;

enum StepGamePlayer1 {Off, go, finish };
int StepGamePlayer1(int state) {
    static unsigned short currentDistance = 0;
    static const unsigned short raceDistance = 20;

    state = go;
    // switch (state) {
    //     case Off : 
    //         if (player1 == 0) 
    //             state = Off;
    //         else {
    //             player1finish = 0;
    //             state = go;
    //         }
    //         break;
    //     case go : 
    //         if (currentDistance >= raceDistance)
    //             state = finish;
    //         else 
    //             state = go;
    //         break;
    //     case finish :
    //         state = Off;
    //         break;
    //     default : 
    //         state = Off; 
    //         break;
    // }

            unsigned char score[3] = {0,0};
    switch (state) {
        case go : 
            if (currentDistance < raceDistance) {
                // if (currentDistance%2 == 0 && isLeftFoot())
                if (isLeftFoot()) {
                    PORTB = 0x01;
                    LCD_DisplayString(1, "left foot");
                    // currentDistance++;
                } else if (isRightFoot()) {
                    PORTB = 0x02;
                    LCD_DisplayString(1, "right foot");
                    // currentDistance++;
                }
            }
            
            // LCD_ClearScreen();
            // score[0] = '0' + (unsigned char) ADC;
            // score[1] = '-';
            // LCD_DisplayString(4, score);
            break;
        case finish:
            player1finish = 1;
            break;
    }
    return state;
}


unsigned char LCD_Text[];
// writes Game text to the LCD Display
enum LCDTextStates { Display };
int LCDTextTick(int state) {
    static int time = 0;
    switch (state) {
        case Display:
            // LCD_DisplayString(1, pot + '0');
            // LCD_DisplayString(1, LCD_Text);
            if (player1finish)
                LCD_DisplayString(1,"Player 1 finished");
            break;
        default: state = Display; break;
    }

    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00;// PORTA = 0x00;
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xff;

    unsigned char period = 50;
    /*
    * TASKS
    */
    int numTasks = 2;
    int i = 0;
    task tasks [numTasks];

    // player1 task
    tasks[i].state = 1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &StepGamePlayer1;
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

player1 = 1;

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
