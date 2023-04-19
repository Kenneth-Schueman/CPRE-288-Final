/*
 * @Author Alexander Moeller, Kenneth Schueman, Clayton Reitz, and Nicholas Pinnello.
 */

volatile char uart_data;
volatile char flag;

#include "uart_extra_help.h"
#include "timer.h"

void uart_init(int baud) {
    SYSCTL_RCGCGPIO_R |= 0b10;      // enable clock GPIOB
    SYSCTL_RCGCUART_R |= 0b10;      // enable clock UART1
    GPIO_PORTB_AFSEL_R = 0b11;      // sets PB0 and PB1 as peripherals
    GPIO_PORTB_PCTL_R  = 0x11;       // pmc0 and pmc1
    GPIO_PORTB_DEN_R   = 0b11;        // enables pb0 and pb1
    GPIO_PORTB_DIR_R   = 0b01;        // sets pb0 as output, pb1 as input

    //compute baud values [UART clock= 16 MHz] 
    double fbrd;
    int    ibrd;

    //brd = 8.6806
    fbrd = 44;
    ibrd = 8;

    UART1_CTL_R &= 0b0;      // disable UART1
    UART1_IBRD_R = ibrd;        // write integer portion of BRD to IBRD
    UART1_FBRD_R = fbrd;   // write fractional portion of BRD to FBRD
    UART1_LCRH_R = 0b1100000;        // write serial communication parameters * 8bit and no parity
    UART1_CC_R   = 0x0;          // use system clock as clock source
    UART1_CTL_R |= 0b1100000001;        // enable UART1

}

void uart_sendChar(char data) {
   while ((UART1_FR_R & 0x20) != 0) {
   }
   
   UART1_DR_R = data;
}

char uart_receive(void) {
    char rdata;

    while ((UART1_FR_R & 0x10) != 0) {
    }
 
    rdata = (char) UART1_DR_R & 0xFF;
    return rdata;
}

void uart_sendStr(const char *data) {
    int i = 0;
    while (data[i] != '\0') {
        uart_sendChar(data[i]);
        i++;
    }
	
}

// _PART3


void uart_interrupt_init() {
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b10000; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    NVIC_EN0_R |= 0b1000000; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number

}

void uart_interrupt_handler() {
    //Check the Masked Interrupt Status
    if (UART1_MIS_R & 0b10000) {
        flag = 1;
    }

    //Copy the data
    uart_data = uart_receive();

    //Clear the interrupt
    UART1_ICR_R |= 0b10000;
}
