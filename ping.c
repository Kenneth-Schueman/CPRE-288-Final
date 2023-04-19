#include "lcd.h"
#include "ping.h"
#include "Timer.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"

volatile enum {LOW, HIGH, DONE} state;  //Set by ISR
volatile unsigned int rising_time;  //Pulse start time
volatile unsigned int falling_time; //Pulse end time
volatile unsigned int overflow = 0; //Overflow tracker

void ping_init(void) {
    SYSCTL_RCGCTIMER_R |= 0x08; //Enable Timer 3 clock
    SYSCTL_RCGCGPIO_R |= 0x2;   //Enable Clock B
    GPIO_PORTB_AFSEL_R |= 0x8;
    GPIO_PORTB_PCTL_R |= 0x7000;
    GPIO_PORTB_PCTL_R &= ~0x8000;
    GPIO_PORTB_DEN_R |= 0b1000;
    GPIO_PORTB_DIR_R |= 0x8;    //Set PB3 as output

    TIMER3_CTL_R &= ~0x101; //Disable Timer 3
    TIMER3_CFG_R |= 0x4;
    TIMER3_TBMR_R &= ~0x10;
    TIMER3_TBMR_R |= 0x7;   //Timer counts down
    TIMER3_CTL_R |= 0xC00;
    TIMER3_TBPR_R |= 0xFF;
    TIMER3_TBILR_R |= 0xFFFF;
    TIMER3_IMR_R |= 0x400;  //Enable interrupt
    TIMER3_ICR_R |= 0x400;  //Clear interrupt
    TIMER3_CTL_R |= 0xD00;  //Enable Timer 3

    NVIC_EN1_R |= 0x10; //Enable interrupt
    NVIC_PRI9_R |= 0x20;    //Interrupt priority

    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    IntMasterEnable();
}

double ping_read(void) {
    double distance = 0.0;
    send_pulse();

    //Wait for ISR to capture rising edge and falling edge time
    while (state != DONE) {}

    if (rising_time < falling_time) {
        int difference;
        difference = 16777215 - falling_time;
        difference += rising_time;
        overflow += 1;
        distance = 34000 * ((1/16000000.0) * (difference / 2));
    } else {
        distance = 34000 * ((1/16000000.0) * (rising_time - falling_time) / 2);
    }

    return distance;
}

void send_pulse() {
    TIMER3_CTL_R &= ~0x100;        //Disable timer
    TIMER3_IMR_R &= ~0x400;       //Disable interrupt
    GPIO_PORTB_AFSEL_R &= ~0x8;       //Turn off alternate function
    GPIO_PORTB_DIR_R |= 0x8;         //Set PB3 as output
    GPIO_PORTB_DATA_R |= 0x8;        //Set PB3 to high
    state = HIGH;
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~0x8;        //Set PB3 to low
    state = LOW;
    GPIO_PORTB_DIR_R &= ~0x8;         //Set PB3 as input
    GPIO_PORTB_AFSEL_R |= 0x8;       //Turn on alternate function
    TIMER3_ICR_R |= 0x400;      //Clear interrupt
    TIMER3_IMR_R |= 0x400;      //Enable interrupt
    TIMER3_CTL_R |= 0x100;       //Enable timer
}

void TIMER3B_Handler(void) {
    if (state == LOW) {
        state = HIGH;
        rising_time = TIMER3_TBR_R;
        TIMER3_ICR_R |= 0x400;
    }
    else if (state == HIGH) {
        falling_time = TIMER3_TBR_R;
        state = DONE;
        TIMER3_ICR_R |= 0x400;
    }
}

int get_pulse() {
    return (rising_time - falling_time);
}

double get_pulse_millis() {
    return ((1/16000000.0) * (rising_time - falling_time) / 2) * 1000.0;
}

int get_overflow() {
    return overflow;
}
