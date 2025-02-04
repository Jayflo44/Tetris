#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "button.h"
#include "buzzer.h"
#include "led.h"
#include <stdio.h>
#include "music.h"
#include "notes.h"
//define game constants
#define GRID_WIDTH 10
#define GRID_HEIGHT 20
#define BLOCK_SIZE 5
  
//define timer setting
#define GAME_TIMER_INTERVAL 250 //Game tick in milliseonds 
#define MUSIC_TIMER_INTERVAL 500 // music tick interval in milliseconds

//Tetrimino shapes and rotation states using structs
typedef struct{
  int x;
  int y; //position of the grid 
  char shape[4][4];// 4X4 frid for the shape
} Tetrimino;

// Function prototypes
void init_game();
void draw_grid();
void draw_tetrimino(Tetrimino* tetrimino);
void move_tetrimino_down();
void rotate_tetrimino();
void place_tetrimino();
void handle_collision();
void clear_full_rows();
void music_update();
void wdt_c_handler();
void led_flash();
void display_score();
void check_game_over();
void button_interrupt_handler();
//Game state variables
volatile short redrawScreen =1;
Tetrimino currentTetrimino;
char gameGrid[GRID_WIDTH][GRID_HEIGHT]; //0 = empty, 1 = occupied
char score = 0;
unsigned int  currentGameTimerInterval = GAME_TIMER_INTERVAL; // Initialize with the constant value
//Music variables
volatile short musicFlag =0;
short musicStep = 0;

//Main function
void main(){
  //setup hardware and peripherals
    configureClocks();
    lcd_init();
    switch_init();
    buzzer_init();
    enableWDTInterrupts();
    init_game();
    clearScreen(COLOR_BLACK);
  // Main game loop
    while (1) {
        if (redrawScreen) {
	  redrawScreen = 0;
          draw_grid();
          draw_tetrimino(&currentTetrimino);
	  display_score();
	}
        if (musicFlag) {
          musicFlag = 0;
          music_update();
        }
      or_sr(0x10);  // CPU OFF
     }
}
// Flash LEDs to indicate game over                                                                                      
void led_flash(int count) {
  for (int i = 0; i < count; i++) {
    P1OUT |= LEDS;  // Turn on LEDs                                                                                      
    __delay_cycles(500000);  // Delay                                                                                    
    P1OUT &= ~LEDS;  // Turn off LEDs                                                                                    
    __delay_cycles(500000);  // Delay                                                                                    
  }
}
void show_menu() {
  clearScreen(COLOR_GREEN);
  drawString5x7(screenWidth / 2 - 30, screenHeight / 2 - 10, "BRICKBREAKER!!", COLOR_BLACK, COLOR_GREEN);
  drawString5x7(screenWidth / 2 - 50, screenHeight / 2 + 20, "Press SW1 to Start", COLOR_BLACK, COLOR_GREEN);
  play_start_screen_music();
  while (1) {
    int currentSwitches = get_switches();
    if (currentSwitches & SW1) {  // Start the game when SW1 is pressed		 
	clearScreen(COLOR_BLUE);   // Clear menu screen
	//turn_on_red();
      break;
    }
  }
}
// Initialize game state
void init_game() {
    // Initialize game grid to empty
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            gameGrid[x][y] = 0;
        }
    }

    // Initialize first Tetrimino
    Tetrimino t = {
      .x = 4,
      .y = 0,
      .shape = {
        {1, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }};
    currentTetrimino = t;
}

// Draw the game grid
void draw_grid() {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            short color = gameGrid[x][y] ? COLOR_GREEN : COLOR_BLACK;
            fillRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, color);
        }
    }
}

// Draw the current Tetrimino
void draw_tetrimino(Tetrimino* tetrimino) {
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (tetrimino->shape[y][x]) {
                fillRectangle(
                    (tetrimino->x + x) * BLOCK_SIZE,
                    (tetrimino->y + y) * BLOCK_SIZE,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    COLOR_RED
                );
            }
        }
    }
}

// Timer interrupt handler
void wdt_c_handler() {
    static int gameTick = 0;
    static int musicTick = 0;

    gameTick++;
    musicTick++;

    if (gameTick >= (currentGameTimerInterval / 10)) {
        gameTick = 0;
        redrawScreen = 1; // Trigger screen redraw
        move_tetrimino_down();
    }

    if (musicTick >= (MUSIC_TIMER_INTERVAL / 10)) {
        musicTick = 0;
        musicFlag = 1; // Trigger music update
    }
}

// Check for collisions with boundaries or other blocks
short check_collision() {
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (currentTetrimino.shape[y][x]) { // Only check filled blocks
                int gridX = currentTetrimino.x + x;
                int gridY = currentTetrimino.y + y;

                // Check boundaries
                if (gridX < 0 || gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) {
                    return 1;
                }

                // Check collision with existing blocks
                if (gridY >= 0 && gameGrid[gridX][gridY]) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Move the current Tetrimino down

void move_tetrimino_down() {
    currentTetrimino.y++;
    if (check_collision()) {
        currentTetrimino.y--; // Revert movement
        place_tetrimino();   // Place the Tetrimino
    }
}
// Rotate the Tetrimino clockwise
void rotate_tetrimino() {
    char temp[4][4];
    char original[4][4];

    // Store the original shape
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            original[x][y] = currentTetrimino.shape[x][y];
        }
    }

    // Perform rotation: Transpose and reverse rows
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            temp[x][y] = currentTetrimino.shape[3 - y][x];
        }
    }

    // Copy rotated shape to current Tetrimino
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            currentTetrimino.shape[x][y] = temp[x][y];
        }
    }

    // Check for collisions after rotation
    if (check_collision()) {
        // Revert to the original shape if collision occurs
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                currentTetrimino.shape[x][y] = original[x][y];
            }
        }
    }
}
// Place the Tetrimino on the grid
void place_tetrimino() {
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (currentTetrimino.shape[y][x]) {
                gameGrid[currentTetrimino.x + x][currentTetrimino.y + y] = 1;
            }
        }
    }

    // Check for full rows
    clear_full_rows();

    //Check for game over
    check_game_over();
    // Generate a new Tetrimino
     Tetrimino t = {
      .x = 4,
      .y = 0,
      .shape = {
        {1, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }};
     currentTetrimino = t;
}

// Clear full rows and shift rows down
void clear_full_rows() {
    for (int y = GRID_HEIGHT - 1; y >= 0; ) { // Start from the bottom row
        short isFull = 1;
	
        // Check if the row is full
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (!gameGrid[x][y]) {
                isFull = 0;
                break;
            }
        }

        if (isFull) {
            // Clear the row and shift rows above down
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    gameGrid[x][yy] = gameGrid[x][yy - 1];
                }
            }

            // Clear the top row (now empty after shifting)
            for (int x = 0; x < GRID_WIDTH; x++) {
                gameGrid[x][0] = 0;
            }
	    score += 10; // Add points for clearing a row
            // Re-check the current row, as rows have shifted
            continue;
        }

        // Move to the next row up
        y--;
    }
}

void button_interrupt_handler() {
  int currentSwitches = get_switches();
  if (currentSwitches & SW1) { // Move left
        currentTetrimino.x--;
        if (check_collision()) {
            currentTetrimino.x++; // Undo move if collision
        }
       redrawScreen = 1;
    }
    if (currentSwitches & SW2) { // Move right
        currentTetrimino.x++;
        if (check_collision()) {
            currentTetrimino.x--; // Undo move if collision
        }
      redrawScreen = 1;
    }
    if (currentSwitches & SW3) { // Rotate
        rotate_tetrimino();
	redrawScreen = 1;
    }
    if (currentSwitches & SW4) { // Drop faster
        move_tetrimino_down();
	redrawScreen = 1;
    } 
}
// Enhanced music update
void music_update() {
    static const short melody[] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5};
    static const short durations[] = {200, 200, 200, 200};
    buzzer_set_period(melody[musicStep]);
    musicStep = (musicStep + 1) % 4;
}
// Display the win screen
void show_win_screen() {
  clearScreen(COLOR_GREEN);
  clearScreen(COLOR_RED);
  clearScreen(COLOR_BLUE);
  clearScreen(COLOR_YELLOW);
  clearScreen(COLOR_PURPLE);
  drawString5x7(screenWidth / 2 - 30, screenHeight / 2 - 10, "YOU WON!", COLOR_WHITE, COLOR_PURPLE);
  led_flash(5);
  play_winning_sound(); // Play the winning sound
  while (1) {
    int currentSwitches = get_switches();
    if (currentSwitches & SW4) {  // Exit the game when SW4 is pressed
      clearScreen(COLOR_BLACK);  // Clear screen
      break;
    }
  }
}
// Handle game over
void game_over() {
  clearScreen(COLOR_BLACK);
  drawString8x12(screenWidth / 2 - 40, screenHeight / 2 - 6, "GAME OVER", COLOR_WHITE, COLOR_BLACK);
  led_flash(5);  // Flash LEDs 5 times
  play_game_over_sound();
  while (1);  // Halt the game
}
// Check for game over condition                                                                                        
void check_game_over() {
    for (int x = 0; x < GRID_WIDTH; x++) {
        if (gameGrid[x][0]) { // If the top row is occupied                                                             
            game_over();      // Trigger game over sequence                                                             
            return;
        }
    }
}
void display_score() {
    char buffer[16];
    sprintf(buffer, "Score: %d", score);
    drawString5x7(10, 10, buffer, COLOR_WHITE, COLOR_BLACK);
}
void update_game_speed() {
    if (score % 50 == 0 && currentGameTimerInterval > 100) {
        currentGameTimerInterval -= 10; // Increase speed every 50 points
    }
}
// Interrupt handler for switch inputs
void __interrupt_vec(PORT2_VECTOR) Port_2() {
  if (P2IFG & SWITCHES) {
    P2IFG &= ~SWITCHES;  // Clear interrupt flags
    button_interrupt_handler(); 
  }
}
