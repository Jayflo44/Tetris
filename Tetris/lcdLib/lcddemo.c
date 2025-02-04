/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;
  
  clearScreen(COLOR_BLACK);
  int col = 0; int row =0;
  int centercol = 50;  // Center X-coordinate
  int centerrow = 50;  // Center Y-coordinate
  int size = 10;       // Size of the octagon "radius"
  for (int row = -size; row <= size; row++) {
    for (int col = -size; col <= size; col++) {
        // Calculate the absolute distances from the center
        int absRow = row > 0 ? row : -row;
        int absCol = col > 0 ? col : -col;
        // Define the octagon boundaries based on absolute distances
        if ((absRow + absCol <= size) && (absRow <= size - 2) && (absCol <= size - 2)) {
            drawPixel(centercol + col, centerrow + row, COLOR_BLUE);
        }
    }
} 
  // drawString5x7(20,20, "hey", COLOR_GREEN, COLOR_RED);

  //fillRectangle(30,30, 10, 10, COLOR_RED);
  
}
