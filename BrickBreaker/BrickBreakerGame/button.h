#ifndef BUTTON_H
#define BUTTON_H
#define SW1 BIT0  // P2.0                                                                                           
#define SW2 BIT1  // P2.1                                                                                           
#define SW3 BIT2  // P2.2                                                                                           
#define SW4 BIT3  // P2.3                                                                                           
#define SWITCHES (SW1 | SW2 | SW3 | SW4)  // Combined mask                                                          
extern volatile int switches;
void switch_init();             // Initialize the switches
void switch_interrupt_handler(); // Handle switch interrupts
int get_switches();             // Get current switch states

#endif // BUTTON_H
