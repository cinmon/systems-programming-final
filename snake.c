/*

    이름: 지아넬라
    학번: 2023030514

    An implementation of the famous Snake
    game for Unix

    In this code, we have two processes.
    - The child processes handles keyboard input
    - The parent process handles game updates

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_SNAKE_SIZE 800
#define SNAKE_HEAD_INDEX 0
#define HEAD_SYMBOL 'O'
#define TAIL_SYMBOL 'o'
#define FRUIT_SYMBOL 'x'

typedef struct {
    int x;
    int y;
} Point;

enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction;

Point snake[MAX_SNAKE_SIZE + 1]; // Accounts for the snake
Point fruit;

int gameOver = false;
int score = 0;
int snake_length = 0;


void setFoodPosition() {
    fruit.x = rand() %  (WIDTH - 2) + 1;
    fruit.y = rand() % (HEIGHT - 2) + 1;
}

void game_initialize() {

    nodelay(stdscr, TRUE);
    noecho();

    srand(time(NULL));

    // Initialize the snake's head in the middle 
    snake[SNAKE_HEAD_INDEX].x = WIDTH / 2;
    snake[SNAKE_HEAD_INDEX].y = HEIGHT / 2;

    // Generate the food location randomly
    setFoodPosition();

}

void draw_screen() {

    static int draw_count = 0;
    void snake_movement();
    
    
    snake_movement();
    clear();

    // Generate the grid
    for(int current_y = 0 ; current_y < HEIGHT ; current_y++) {
        for(int current_x = 0 ; current_x < WIDTH ; current_x++) {
            
            if(current_x == 0  || current_x == WIDTH - 1
            || current_y == 0  || current_y == HEIGHT - 1) {
                mvaddch(current_y, current_x, '#'); 
            }
            else if(current_x == fruit.x && current_y == fruit.y) {
                mvaddch(current_y, current_x, FRUIT_SYMBOL);
            }
            else {
                mvaddch(current_y, current_x, ' ');
            }

        }
    }

    for (int i = 0; i <= snake_length; i++)
    {
        if(i == 0) mvaddch(snake[i].y, snake[i].x, 'O');
        else if (i == snake_length) mvaddch(snake[i].y, snake[i].x, '.');
        else mvaddch(snake[i].y, snake[i].x, 'o');
    }
    

    mvaddch(LINES - 1, COLS - 1, draw_count++ + '0');
    char out[100];
    sprintf(out, "%d, %d", snake[0].x, snake[0].y);
    mvaddstr(LINES - 2, COLS - 50, out);

    refresh();

}

void snake_movement() {
    char input = getch(); 
    /*
        getch is a curses function
        to read a single character or 
        keypress from the user.

        It can handle special characters,
        such as the arrow keys.
    */
    tolower(input);
    
    switch(input){

        case 'w':
            if(direction != DOWN) {
                direction = UP;
            }
            break;

        case 's':
            if(direction != UP) {
                direction = DOWN;
            }
            break;

        case 'a':
            if(direction != RIGHT) {
                direction = LEFT;
            }
            break;
        
        case 'd':
            if(direction != LEFT) {
                direction = RIGHT;
            }
            break;

    }

    for (int i = snake_length; i > 0; i--)
    {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
    }
   
    switch(direction) {

        case UP:
            snake[SNAKE_HEAD_INDEX].y--;
            break;

        case DOWN:
            snake[SNAKE_HEAD_INDEX].y++;
            break;

        case LEFT:
            snake[SNAKE_HEAD_INDEX].x--;
            break;

        case RIGHT:
            snake[SNAKE_HEAD_INDEX].x++;
            break;

    }

    for (int i = 1; i <= snake_length; i++)
    {
        if(snake[SNAKE_HEAD_INDEX].x == snake[i].x && snake[SNAKE_HEAD_INDEX].y == snake[i].y) {
            gameOver = 1;
        }
    }
    
    
    if(snake[SNAKE_HEAD_INDEX].x == fruit.x && snake[SNAKE_HEAD_INDEX].y == fruit.y) {
        setFoodPosition();
        score += 10;
        snake_length += 1;

        switch(direction) {
            case UP:
                snake[snake_length].x = snake[snake_length - 1].x;
                snake[snake_length].y = snake[snake_length - 1].y + 1;
                break;
            case DOWN:
                snake[snake_length].x = snake[snake_length - 1].x;
                snake[snake_length].y = snake[snake_length - 1].y - 1;
                break;
            case LEFT:
                snake[snake_length].x = snake[snake_length - 1].x + 1;
                snake[snake_length].y = snake[snake_length - 1].y;
                break;
            case RIGHT:
                snake[snake_length].x = snake[snake_length - 1].x - 1;
                snake[snake_length].y = snake[snake_length - 1].y;
                break;
        }
    }
    

    if(snake[SNAKE_HEAD_INDEX].x < 1 || snake[SNAKE_HEAD_INDEX].x >= WIDTH  - 1 
    || snake[SNAKE_HEAD_INDEX].y < 1 || snake[SNAKE_HEAD_INDEX].y >= HEIGHT - 1) {
        gameOver = true;
    } 


}

int set_ticker(int n_msecs)
{
	/* configures a timer to issue a SIGALRM 
	signal at regular intervals specified in milliseconds.

	n_msecs is number of milliseconds between SIGALRM signals. 
	This value is used to configure the timer.
	*/

	struct itimerval new_timeset;	// new setting
	long n_sec, n_usecs;	// new sec, usec

	n_sec = n_msecs / 1000;				// converts the milliseconds value into whole seconds.
	n_usecs = (n_msecs % 1000) * 1000L; // calculates the remaining microseconds (1 millisecond = 1000 microseconds).

	new_timeset.it_interval.tv_sec = n_sec;	   /* set reload       */
	new_timeset.it_interval.tv_usec = n_usecs; /* new ticker value */
	new_timeset.it_value.tv_sec = n_sec;	   /* store this       */
	new_timeset.it_value.tv_usec = n_usecs;	   /* and this         */

	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

int main() {

    initscr();

    // keypad(stdscr, TRUE);

    curs_set(0); // to hide the cursor
    game_initialize();

    signal(SIGALRM, draw_screen);

    if(set_ticker(100) == -1) {
        perror("set_ticker");
    }

    while (!gameOver);

    set_ticker(0);
    mvprintw(HEIGHT / 2, WIDTH / 2 - 13, "Game Over! Press X to exit");
    while(tolower(getch()) != 'x');
    
    endwin();

    return 0;

}
