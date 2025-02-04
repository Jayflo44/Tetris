#include <msp430.h>
#include "buzzer.h"
#include "notes.h"

// Initialize the buzzer
void music_init() {
    buzzer_init();
}
void delay_ms(unsigned int ms) {
    while(ms--) {
        __delay_cycles(3000);
    }
}
void play_note_blocking(int frequency, int duration_ms) {
    int period = 1000000 / frequency;  // Calculate period for the buzzer
    buzzer_set_period(period);
    delay_ms(duration_ms);  // Blocking delay (assuming 1 ms per 3000 cycles)
    buzzer_set_period(0);   // Stop the buzzer
    delay_ms(50);           // Short delay between notes
}
volatile int D = 800;  // Default duration for notes

void play_start_screen_music() {
    play_note_blocking(NOTE_C5, D);
    play_note_blocking(NOTE_E5, D);
    play_note_blocking(NOTE_G5, D);
    play_note_blocking(NOTE_C6, 1600);
    play_note_blocking(NOTE_E5, D);
    play_note_blocking(NOTE_C5, D);
    play_note_blocking(NOTE_D5, D);
    play_note_blocking(NOTE_F5, D);
    play_note_blocking(NOTE_A5, 1600);
    play_note_blocking(NOTE_G5, D);
    play_note_blocking(NOTE_E5, 1600);
}
void play_collision_sound() {
    play_note_blocking(NOTE_G4, 800);
    play_note_blocking(NOTE_E4, 800);
    play_note_blocking(NOTE_C4, 800);
    play_note_blocking(NOTE_A3, 800);
    play_note_blocking(NOTE_G3, 1600);
    play_note_blocking(NOTE_F3, 800);
    play_note_blocking(NOTE_E3, 1600);
}
void play_winning_sound() {
    play_note_blocking(NOTE_C5, 500);
    play_note_blocking(NOTE_E5, 500);
    play_note_blocking(NOTE_G5, 500);
    play_note_blocking(NOTE_C6, 1000);
    play_note_blocking(NOTE_G5, 500);
    play_note_blocking(NOTE_E5, 500);
    play_note_blocking(NOTE_C5, 1000);
    play_note_blocking(NOTE_E5, 500);
    play_note_blocking(NOTE_G5, 500);
    play_note_blocking(NOTE_C6, 1600);
}
void play_game_over_sound() {
    play_note_blocking(NOTE_C4, 400);
    play_note_blocking(NOTE_A3, 400);
    play_note_blocking(NOTE_F3, 400);
    play_note_blocking(NOTE_D3, 400);
    play_note_blocking(NOTE_C3, 800);
    play_note_blocking(NOTE_D3, 400);
    play_note_blocking(NOTE_F3, 400);
    play_note_blocking(NOTE_A3, 400);
    play_note_blocking(NOTE_C4, 800);
}
void play_laser_sound() {
    play_note_blocking(NOTE_E6, 100);
}
