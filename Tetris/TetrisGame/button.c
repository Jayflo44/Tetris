#include <msp430.h>
#include "button.h"
#include "led.h"

volatile int switches = 0;  // Define switches here

char switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);  /* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES); /* if switch down, sense up */
  return p2val;
}
void
switch_init()                   /* setup switch */
{
  P2REN |= SWITCHES;            /* enables resistors for switches */
  P2IE |= SWITCHES;             /* enable interrupts from switches */
  P2OUT |= SWITCHES;            /* pull-ups for switches */
  P2IFG &= ~SWITCHES;   // Clear interrupt flags
  P2IES |= SWITCHES;    // Trigger on high-to-low transition
  P2DIR &= ~SWITCHES;           /* set switches' bits for input */
  switch_update_interrupt_sense();
}
void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}

int get_switches() {
  return switches;  // Return current switch states
}
