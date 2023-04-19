#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

/*
 * @Author Alexander Moeller, Kenneth Schueman, Clayton Reitz, and Nicholas Pinnello.
 */

//Initialize the ADC for sampling
void adc_init(void);

//Read in values for sampling
int adc_read(void);
