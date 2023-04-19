#include "Timer.h"
#include "lcd.h"
#include "servo.h"
#include <stdint.h>
#include <stdbool.h>

extern volatile int right_calibration_val;
extern volatile int left_calibration_val;

void servo_init(void) {
    SYSCTL_RCGCTIMER_R |= 0x2; //Enable Timer 1
    SYSCTL_RCGCGPIO_R |= 0x2;  //Enable GPIO Port B clock
    GPIO_PORTB_AFSEL_R |= 0x20; //PB5
    GPIO_PORTB_PCTL_R |= 0x700000;
    GPIO_PORTB_DEN_R |= 0x20; //Enable PB5
    GPIO_PORTB_DIR_R |= 0x20; //Set PB5 to output

    TIMER1_CTL_R &= ~0x100; //Disable Timer 1
    TIMER1_CFG_R |= 0x4;    //16 bit
    TIMER1_TBMR_R |= 0b1010;    //Periodic, edge count, PWM
    TIMER1_TBMR_R &= 0b1010;
    //TIMER1_CTL_R &= 0xBFFF;
    TIMER1_TBPR_R = 0x4;
    TIMER1_TBILR_R = 0xE200;
    TIMER1_TBPMR_R = 0x4;
    TIMER1_TBMATCHR_R = 0xC000;
    TIMER1_CTL_R |= 0x100;  //Enable Timer 1B
}

int servo_move(float degrees) {
    int y = right_calibration_val - (((right_calibration_val - left_calibration_val) / 180.0) * degrees);

    TIMER1_TBPMR_R |= ((320000 - y) >> 16);
    TIMER1_TBMATCHR_R = (y & 0xFFFF);
    timer_waitMillis(200);
    return y;
}
