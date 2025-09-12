/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CanInitializer.h"
#include "PedalSensors.h"
#include <stdio.h>
#include <sys/_intsup.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

#define DEBUGGING 0

// Placeholder values while we do not have real numbers (most likely same as 24 car)
const int ocThreshold = 1; // Open Circuit threshold
const int scThreshold = 2;  // Short Circuit threshold

// Pin Configuration NOT REAL VALUES
const int brake_switch_pin = 14;
const int brake_out_pin = 2;
const int error_out_pin = 9;
const int rtds_horn_pin = 21;
const int custom_pedal_pin = A14;




// Pedal sensor1 WRITE DOWN RIGHT OR LEFT WHEN WE FIGURE IT OUT also update values
const pedalSensor p1 = {
    0,
    0,
    0,
    0,
    0,
    0
};

// Pedal sensor2 WRITE DOWN RIGHT OR LEFT WHEN WE FIGURE IT OUT also update values
const pedalSensor p2 = {
    0,
    0,
    0,
    0,
    0,
    0
};

const pedalSensor[] pedals = [p1, p2];

//******
// This would be where pedal map was defined with old apps
//  */

// If sensors disagree by more than 10% for >100ms, fault.
// DOUBLE CHECK RULES and if millis will work of if that needs a library
bool pedalAgreementFault() {
    static uint32_t faultTime = 0;
    if (abs(pedalPercentage(p1) - pedalPercentage(p2)) > 0.1) { // maybe increase if p2 is being really screwy
    if (faultTime == 0) { 
        faultTime = millis() + 100; // time when apps should start faulting
    } else if (faultTime <= millis()) {
        return true;
    }
    } else {
    faultTime = 0;
  }
  return false;
}



/* The devicetree node identifier for the "led0" alias. */

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

int main(void) {}
