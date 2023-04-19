#include "lcd.h"
#include "adc.h"
#include "Timer.h"

void adc_init(void) {
    SYSCTL_RCGCADC_R |= 0b1;
    SYSCTL_RCGCGPIO_R |= 0b11010;
    GPIO_PORTB_AFSEL_R |= 0b10000;
    GPIO_PORTB_DEN_R &= 0b01111;
    GPIO_PORTB_AMSEL_R |= 0b10000;
    GPIO_PORTB_ADCCTL_R &= 0x0;

    ADC0_ACTSS_R &= 0b0000;
    ADC0_EMUX_R &= 0x0000;
    ADC0_SSMUX0_R |= 0xA;
    ADC0_SSCTL0_R |= 0b110;
    ADC0_SAC_R |= 0x4;
    ADC0_ACTSS_R |= 0b1111;
}

int adc_read(void) {
    //Initialize SS0
    ADC0_PSSI_R |= 0b1;

    //Wait until interrupt occurs
    while (~ADC0_RIS_R & 0b1) {
    }

    //Clear interrupt
    ADC0_ISC_R |= 0b1;
    return ADC0_SSFIFO0_R;
}
