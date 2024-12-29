# systems-programming-final

## Game Description
  

This Snake Game is a simple yet engaging game where:
- A snake moves around the game board, consuming fruits.
- Each fruit eaten increases the snake's length and the player's score.
- The game ends if the snake collides with the walls or itself.


The game uses keyboard inputs (`W`, `A`, `S`, `D`) to control the snake's movement.
  

## Features

This program integrates 5 main features:


1. Shell Scripting Integration: To build and run the program. 
2. Timer Signals: To make periodical game updates.
3. Keyboard Control: To control the snake's movement.
4. Multi-Process Design: To separate game logic and user inputs (total of 2 processes). The parent process updates the game state and the screen, while the child process captures and communicates user inputs.
5. Inter-Process Communication: To enable communication between the two processes.

## Build

  
To build this program, two things have to be installed:
1. A C compiler
2. The `ncurses` library

To compile and run the program, run `sh Makefile.sh`.

Use `W` (up), `A` (left), `S` (down), `D` (right) to navigate. After the game ends, press `X` to exit.
 