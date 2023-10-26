#include <stdio.h>

#ifndef _ADC_H
#define _ADC_H

void adc_setup(void);
uint16_t read_adc_naiive(void);

#define ADC_RES 4095
#define VREF 5

#endif