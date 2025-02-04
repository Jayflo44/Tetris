#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "button.h"
#include "buzzer.h"
#include "led.h"
#include <stdio.h>
#include "music.h"
#define PADDLE_SPEED 2
#define PADDLE_HEIGHT 7
#define PADDLE_MARGIN 5
// Function prototypes
void paddle_update();
void music_init();
void ball_update();
void init_bricks();
void draw_bricks();
void check_collision_with_bricks(short ballX, short ballY);
void switch_interrupt_handler();
void update_shape();
void game_over();
void led_flash(int count);
void show_win_screen();
void turn_on_red();

// Global variables

volatile short redrawScreen = 1;  // Flag to indicate screen updates
short paddlePos = screenWidth / 2;  // Initial paddle position
short paddleWidth = 20;  // Paddle width
short ballPos[2] = {screenWidth / 2, screenHeight / 2};  // Ball position
short ballVelocity[2] = {1, 1};  // Ball velocity

// Brick definitions
#define NUM_BRICKS 20
typedef struct {
  short x, y;  // Position of the brick
  short width, height;  // Dimensions of the brick
  char active;  // 1 if the brick is active, 0 if broken
} Brick;
Brick bricks[NUM_BRICKS];

// Initialize LEDs
void led_init() {
  P1DIR |= LEDS;  // Set LED pins as outputs
  P1OUT &= ~LEDS; // Turn off LEDs
}
// Control LEDs
void led_update() {
    int currentSwitches = get_switches();
    if (currentSwitches & SW2) {
        P1OUT |= LED_GREEN;  // Turn on green LED
    } else {
        P1OUT &= ~LED_GREEN;  // Turn off green LED
    }
    if (currentSwitches & SW3) {
        P1OUT |= LED_RED;  // Turn on red LED
    } else {
        P1OUT &= ~LED_RED;  // Turn off red LED
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
short prevBallPos[2] = {screenWidth / 2, screenHeight / 2};  // Previous ball position                
void wdt_c_handler() {
    static int secCount = 0;
    secCount++;
    if (secCount >= 25) {  // Trigger updates 1/10 times per second
      secCount = 0;
      redrawScreen = 1;  // Flag screen for redraw
       // Move the ball                                                                              
      ballPos[0] += ballVelocity[0];                                                                 
      ballPos[1] += ballVelocity[1];
      //check collision with brick                                                                   
      check_collision_with_bricks(ballPos[0], ballPos[1]);
      // Ball-wall collisions                                                                        
      if (ballPos[0] <= 0 || ballPos[0] >= screenWidth) {
	ballVelocity[0] = -ballVelocity[0];                                                          
      }                                                                                              
      if (ballPos[1] <= 0) {                                                                         
       ballVelocity[1] = -ballVelocity[1];
      }                                                                                          
      // Ball-paddle collision                                                                       
      if (ballPos[1] >= screenHeight - 15 &&                                                         
          ballPos[0] >= paddlePos &&                                                                 
          ballPos[0] <= paddlePos + paddleWidth) {                                                 
          ballVelocity[1] = -ballVelocity[1];
      }   
    }
}
// Initialize bricks
void init_bricks() {
  for (int i = 0; i < NUM_BRICKS; i++) {
    bricks[i].x = (i % 5) * 20 + 10;  // Columns
    bricks[i].y = (i / 5) * 10 + 10;  // Rows
    bricks[i].width = 15;
    bricks[i].height = 5;
    bricks[i].active = 1;
  }
}
// Draw bricks on the screen
void draw_bricks() {
  for (int i = 0; i < NUM_BRICKS; i++) {
    if (bricks[i].active) {
      fillRectangle(bricks[i].x, bricks[i].y, bricks[i].width, bricks[i].height, COLOR_GREEN);
    }
  }
}
// Check collision with bricks
void check_collision_with_bricks(short ballX, short ballY) {
    int allBricksCleared = 1;  // Assume all bricks are cleared unless proven otherwise
    for (int i = 0; i < NUM_BRICKS; i++) {
        if (bricks[i].active) {
            allBricksCleared = 0;  // At least one brick is still active
            // Check if ball overlaps with brick boundaries
            if (ballX + 1 >= bricks[i].x && ballX - 1 <= bricks[i].x + bricks[i].width &&
                ballY + 1 >= bricks[i].y && ballY - 1 <= bricks[i].y + bricks[i].height) {
                // Collision detected
                bricks[i].active = 0;  // Deactivate brick
                fillRectangle(bricks[i].x, bricks[i].y, bricks[i].width, bricks[i].height, COLOR_BLUE);  // Clear brick
                ballVelocity[1] = -ballVelocity[1];  // Reverse vertical direction
                // Play collision sound
		play_laser_sound();
                break;  // Exit loop after first collision
            }
        }
    }
    if (allBricksCleared) {
        show_win_screen();  // Show win screen if all bricks are cleared
    }
}
// Update ball position
void ball_update() {
  // Update ball position
  ballPos[0] += ballVelocity[0];
  ballPos[1] += ballVelocity[1];
  // Ball-wall collision
  if (ballPos[0] <= 0 || ballPos[0] >= screenWidth) {
    ballVelocity[0] = -ballVelocity[0];  // Reverse horizontal direction
  }
  if (ballPos[1] <= 0) {
    ballVelocity[1] = -ballVelocity[1];  // Reverse vertical direction
  }
  if (ballPos[1] >= screenHeight) {
    game_over();  // End game if the ball hits the bottom
  }
  // Ball-paddle collision
  if (ballPos[1] >= screenHeight - 15 &&
      ballPos[0] >= paddlePos &&
      ballPos[0] <= paddlePos + paddleWidth) {
      ballVelocity[1] = -ballVelocity[1];  // Reverse vertical direction
      buzzer_set_period(10500);  // Play sound on paddle hit
      __delay_cycles(200000);  // Short delay
      buzzer_set_period(0);  // Turn off sound
  }
  // Draw the ball
  fillRectangle(ballPos[0] - 1, ballPos[1] - 1, 3, 3, COLOR_RED);
}
//draw paddles 
void paddle_update() {
    int currentSwitches = get_switches();  // Fetch switch states                                 
    fillRectangle(paddlePos, screenHeight - PADDLE_MARGIN, paddleWidth, PADDLE_HEIGHT, COLOR_BLUE);
    if ((currentSwitches & SW2) && paddlePos > 0) {
        paddlePos -= PADDLE_SPEED;  // Move left                                                       
    }
    if ((currentSwitches & SW3) && paddlePos < screenWidth - paddleWidth ) {
        paddlePos += PADDLE_SPEED;  // Move right                                                      
    }
    fillRectangle(paddlePos, screenHeight - PADDLE_MARGIN, paddleWidth, PADDLE_HEIGHT, COLOR_WHITE);
}
// Update the screen
void update_shape() {
    int currentSwitches = get_switches();  // Fetch switch states
    // Update paddle position
    paddle_update();
    // Redraw bricks                                                                                 
    draw_bricks();
    // Update ball position
    if (ballPos[0] != prevBallPos[0] || ballPos[1] != prevBallPos[1]) {
        // Clear old ball position
        fillRectangle(prevBallPos[0] - 1, prevBallPos[1] - 1, 3, 3, COLOR_BLUE);
        // Move ball and check for collisions
        ballPos[0] += ballVelocity[0];
        ballPos[1] += ballVelocity[1];
        check_collision_with_bricks(ballPos[0], ballPos[1]);
        // Ball-wall collision
        if (ballPos[0] <= 0 || ballPos[0] >= screenWidth) {
            ballVelocity[0] = -ballVelocity[0];
        }
        if (ballPos[1] <= 0) {
            ballVelocity[1] = -ballVelocity[1];
        }
        if (ballPos[1] >= screenHeight) {
            game_over();
        }
        // Ball-paddle collision
        if (ballPos[1] >= screenHeight - 15 &&
            ballPos[0] >= paddlePos &&
            ballPos[0] <= paddlePos + paddleWidth) {
            ballVelocity[1] = -ballVelocity[1];
        }
        // Draw new ball position
        fillRectangle(ballPos[0] - 1, ballPos[1] - 1, 3, 3, COLOR_RED);
        // Update previous ball position
        prevBallPos[0] = ballPos[0];
        prevBallPos[1] = ballPos[1];
    }
}

// Display the menu screen                                                                         
void show_menu() {
  clearScreen(COLOR_GREEN);
  drawString5x7(screenWidth / 2 - 30, screenHeight / 2 - 10, "BRICKBREAKER!!", COLOR_BLACK, COLOR_GREEN);                       
  drawString5x7(screenWidth / 2 - 50, screenHeight / 2 + 20, "Press SW1 to Start", COLOR_BLACK, COLOR_GREEN);                     
  //drawString8x12(screenWidth / 2 - 30, screenHeight / 2 - 10, "BRICKBREAKER!!", COLOR_WHITE, COLOR_BLUE);
  //drawString8x12(screenWidth / 2 - 50, screenHeight / 2 + 20, "Press SW1 to Start", COLOR_WHITE, COLOR_BLUE);
  play_start_screen_music();
  while (1) {
    int currentSwitches = get_switches();
    if (currentSwitches & SW1) {  // Start the game when SW1 is pressed                               
      clearScreen(COLOR_BLUE);   // Clear menu screen
      turn_on_red();
      break;
    }
  }
}

// Display the win screen
void show_win_screen() {
  clearScreen(COLOR_GREEN);
  clearScreen(COLOR_RED);
  clearScreen(COLOR_BLUE);
  clearScreen(COLOR_YELLOW);
  clearScreen(COLOR_PURPLE);
  drawString5x7(screenWidth / 2 - 30, screenHeight / 2 - 10, "YOU WON!", COLOR_WHITE, COLOR_PURPLE);
  led_flash(50);
  play_winning_sound(); // Play the winning sound
  while (1) {
    int currentSwitches = get_switches();
    if (currentSwitches & SW4) {  // Exit the game when SW4 is pressed
      clearScreen(COLOR_BLACK);  // Clear screen
      init_bricks();  // Reinitialize bricks
      ballPos[0] = screenWidth / 2;
      ballPos[1] = screenHeight / 2;
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

// Main function
void main() {
  // Setup
  P1DIR |= LEDS;
  P1OUT &= ~LEDS;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  music_init();
  led_init();
  init_bricks();
  clearScreen(COLOR_BLUE);
  enableWDTInterrupts();
  or_sr(0x8);  // Enable interrupts
   // Show the menu screen
  show_menu();

  //main loop
  while(1){
  // CPU OFF: Execution will rely on interrupts.
   if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
      __delay_cycles(1000000);  // Adjust delay for game speed                                       
    }
   or_sr(0x10);  // CPU OFF
  }
}

// Interrupt handler for switch inputs
void __interrupt_vec(PORT2_VECTOR) Port_2() {
    if (P2IFG & SWITCHES) {
        P2IFG &= ~SWITCHES;  // Clear interrupt flags
	switch_interrupt_handler();
    }
}
