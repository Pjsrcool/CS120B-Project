/*	Author: justin
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "../header/io.h"
#include "../header/scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char LCD_Text[];

// writes Game text to the LCD Display
enum LCDTextStates { Display };
int LCDTextTick(int state) {
    static int time = 0;
    switch (state) {
        case Display:
            LCD_DisplayString(1, LCD_Text);
            break;
        default: state = Display; break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xff;
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xff;
    /*
    * TASKS
    */
    int numTasks = 2;
    int i = 0;
    task tasks [numTasks];

    // LCD Text
    tasks[i].state = -1;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &LCDTextTick;
    i++;
    // Output

    LCD_init();
    while (1) {

    }
    return 1;
}
