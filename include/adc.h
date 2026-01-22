#pragma once
#include "zephyr-common.h"
#include "zephyr/kernel.h"


int init_adc(void);
float get_voltage(void);          // Simple - does ADC read & conversion internally
int16_t get_adc_raw_value(void);  // Get raw value from last read

int16_t get_pedal_current(void); 
