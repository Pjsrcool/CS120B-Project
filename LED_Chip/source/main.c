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

unsigned char LCD_Text[];

// writes Game text to the LCD Display
enum LCDTextStates { Display };
int LCDTextTick(int state) {
    // for testing
    unsigned char pot = (unsigned char) ADC;
    // LCD_DisplayString(1, pot);
    static int time = 0;
    switch (state) {
        case Display:
            // LCD_DisplayString(1, pot + '0');
            // LCD_DisplayString(1, LCD_Text);
            LCD_ClearScreen();
            LCD_WriteData(pot + '0');
            break;
        default: state = Display; break;
    }

    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xff;
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xff;

    unsigned char period = 50;
    /*
    * TASKS
    */
    int numTasks = 1;
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
