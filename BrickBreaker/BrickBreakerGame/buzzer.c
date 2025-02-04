#include "libTimer.h"
#include "buzzer.h"
#include <msp430.h>

void buzzer_init() {
    timerAUpmode();
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7;
    P2SEL |= BIT6;
    P2DIR = BIT6;  // Enable output to speaker (P2.6)
}

void buzzer_set_period(short cycles) {
    if (cycles == 0) {
        // Stop the timer output by setting CCR0 and CCR1 to 0
        CCR0 = 0;
        CCR1 = 0;
    } else {
        // Set the timer period to create the desired frequency
        CCR0 = cycles;
        CCR1 = cycles >> 1;  // Set duty cycle to 50%
    }
}
