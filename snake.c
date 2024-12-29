#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_SNAKE_SIZE 800
#define SNAKE_HEAD_INDEX 0
#define HEAD_SYMBOL 'O'
#define TAIL_SYMBOL 'o'
#define FRUIT_SYMBOL 'x'

#define READING_END 0
#define WRITING_END 1

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

Point snake[MAX_SNAKE_SIZE + 1];
Point fruit;

int gameOver = 0;
int score = 0;
int snake_length = 0;
int thePipe[2]; // Pipe file descriptors

void setFoodPosition() {
    fruit.x = rand() % (WIDTH - 2) + 1;
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
    clear();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                mvaddch(y, x, '#');
            } else if (x == fruit.x && y == fruit.y) {
                mvaddch(y, x, FRUIT_SYMBOL);
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }

    for (int i = 0; i <= snake_length; i++) {
        mvaddch(snake[i].y, snake[i].x, (i == 0) ? HEAD_SYMBOL : TAIL_SYMBOL);
    }

    char score_msg[50];
    sprintf(score_msg, "Score: %d", score);
    mvaddstr(HEIGHT, 2, score_msg);
    refresh();
}

void snake_movement() {

    for (int i = snake_length; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    switch (direction) {

        case UP:    
            snake[0].y--; 
            break;

        case DOWN:  
            snake[0].y++; 
            break;
            
        case LEFT:  
            snake[0].x--; 
            break;

        case RIGHT: 
            snake[0].x++; 
            break;

    }

    if (snake[0].x == fruit.x && snake[0].y == fruit.y) {
        score += 10;
        snake_length++;
        setFoodPosition();
    }

    if (snake[0].x <= 0 || snake[0].x >= WIDTH - 1 ||
        snake[0].y <= 0 || snake[0].y >= HEIGHT - 1) {
        gameOver = 1;
    }

    for (int i = 1; i <= snake_length; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            gameOver = 1;
        }
    }
}

void update_game(int signum) {
    snake_movement();
    draw_screen();
}

int set_ticker(int n_msecs) {
    struct itimerval new_timeset;
    long n_sec, n_usecs;

    n_sec = n_msecs / 1000;
    n_usecs = (n_msecs % 1000) * 1000L;

    new_timeset.it_interval.tv_sec = n_sec;
    new_timeset.it_interval.tv_usec = n_usecs;
    new_timeset.it_value.tv_sec = n_sec;
    new_timeset.it_value.tv_usec = n_usecs;

    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

int main() {
    
    if (pipe(thePipe) == -1) {
        perror("pipe");
        return 1;
    }

    initscr();
    curs_set(0); // to hide the cursor
    game_initialize();

    int pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(0);
        return 1;
    }

    if (pid == 0) {

        /*
            The child process will handle input
        */

        close(thePipe[READING_END]);  // the child process will only write

        char input;

        while (!gameOver) {
            input = tolower(getch());
            if (write(thePipe[WRITING_END], &input, sizeof(input)) == -1) {
                perror("write");
                break;
            }
        }
        close(thePipe[1]);

    } else {

        /*
            The parent process will update the screen
            and handle the game logic
        */

        close(thePipe[WRITING_END]); // Close write end

        signal(SIGALRM, update_game);
        set_ticker(100);

        char input;

        while (!gameOver) {

            if (read(thePipe[READING_END], &input, sizeof(input)) > 0) {

                switch (input) {
                case 'w':
                    if (direction != DOWN) {
                        direction = UP;
                    }
                    break;
                case 's':
                    if (direction != UP) {
                        direction = DOWN;
                    }
                    break;
                case 'a':
                    if (direction != RIGHT) {
                        direction = LEFT;
                    }
                    break;
                case 'd':
                    if (direction != LEFT) {
                        direction = RIGHT;
                    }
                    break;
                }

            }

        }

        close(thePipe[READING_END]);
        set_ticker(0); // turn off the alarm

        mvprintw(HEIGHT / 2, WIDTH / 2 - 13, "Game Over! Press X to exit");
        while (tolower(getch()) != 'x');
        
    }

    endwin();
    return 0;
}
