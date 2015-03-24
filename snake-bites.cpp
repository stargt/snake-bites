#ifdef _WIN64
    #include <curses.h>
	#include <windows.h>
	#include <time.h>
#elif _WIN32
    #include <curses.h>
	#include <windows.h>
	#include <time.h>
#elif __APPLE__
    #include <curses.h>
	#include <unistd.h>
	#include <string.h>
	#include <stdlib.h>
	#include <time.h>
    #if TARGET_IPHONE_SIMULATOR
    #elif TARGET_OS_IPHONE
    #elif TARGET_OS_MAC
    #else
    #endif
#elif __linux
	/* Not yet tested on this platform */
	#include <curses.h>
	#include <unistd.h>
	#include <string.h>
#elif __unix
	/* Not yet tested on this platform */
	#include <curses.h>
	#include <unistd.h>
	#include <string.h>
#elif __posix
	/* Not yet tested on this platform */
	#include <curses.h>
	#include <unistd.h>
	#include <string.h>
#endif

#define MAX_BODY_LENGTH 100
#define INITIAL_BODY_LENGTH 3 // Simply change initial snake's length of body
#define NUMBER_OF_APPLE 10
#define SCORE_PER_APPLE 100

enum DIRECTION { UP = 0, RIGHT, DOWN, LEFT };
enum RESULT { WIN, LOSE };

/* Structure for storing x-y position */
typedef struct pos {
	int y;
	int x;
} pos_t;

int kbhit(void);
void wait_for_milliseconds(int);
void shift_right_array(pos_t*, int);

int main(int argc, char *argv[])
{
	/* Initiate screen */
	initscr();
	clear();
	noecho();
	cbreak();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	curs_set(0);
	start_color();

	/* Initiate color pairs */
	init_pair(1, COLOR_RED, COLOR_RED);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);

	/* Initiate variables */
	int number_of_eaten_apple = 0;
	int center_y = LINES / 2;
	int center_x = COLS / 2;
	int next_y = center_y; // Set center of the screen as start position of snake's head
	int next_x = center_x; // Set center of the screen as start position of snake's head
	enum DIRECTION next_direction = RIGHT; // Direction for next movement of snake
	enum RESULT result; // Result of game
	int body_length = INITIAL_BODY_LENGTH;
	pos_t body[MAX_BODY_LENGTH]; // Dot positions consisting body of snake

	/* Print some texts and frames */
	char title_text[] = "Snake Bites Game by JONG HYUCK LIM (Total size: %dw x %dh)";
	char key_guide_text[] = "Available keys: A, D or <-, ->";
	attron(COLOR_PAIR(3));
	mvprintw(0, (COLS - strlen(title_text))/2, title_text, COLS, LINES);
	attroff(COLOR_PAIR(3));
	attron(COLOR_PAIR(2));
	mvprintw(LINES - 1, COLS - strlen(key_guide_text), key_guide_text, COLS, LINES);
	attroff(COLOR_PAIR(2));
	attron(COLOR_PAIR(1));
	for (int i = 0; i < COLS; i++)
	{
		mvaddch(1, i, 'N');
		mvaddch(LINES - 2, i, 'N');
	}
	for (int i = 2; i < LINES - 2; i++)
	{
		mvaddch(i, 0, 'N');
		mvaddch(i, COLS - 1, '#');
	}
	attroff(COLOR_PAIR(1));
	refresh();

	/* Set snake's initial body parts */
	for (int i = 0; i < body_length; i++)
	{
		pos_t temp_pos = { next_y, next_x - (1 + i) };
		body[i] = temp_pos; // (pos_t) { next_y, next_x - (1 + i) }; (M$ Visual Studio sucks that even it doesn't understand this standard C/C++ code. That's why I put temp_pos.)
		mvaddch(body[i].y, body[i].x, '#');
	}

	/* Print apples */
	int temp_x, temp_y;
	srand((unsigned long) time(NULL));
	for (int i = 0; i < NUMBER_OF_APPLE; i++)
	{
		temp_y = rand()%(LINES - 4) + 2;
		temp_x = rand()%(COLS - 2) + 1;
		
		if ((temp_y != next_y && temp_x != next_x) && mvinch(next_y, next_x) != '#')
			mvaddch(temp_y, temp_x, 'A');
		else
			i--;
	}
	
	/* Main loop */
	while (1)
	{
		/* Print score */
		attron(COLOR_PAIR(2));
		mvprintw(LINES - 1, 0, "Score: %d", number_of_eaten_apple * SCORE_PER_APPLE);
		attroff(COLOR_PAIR(2));

		/* Draw snake */
		mvaddch(next_y, next_x, '@'); // Print snake's head
		mvaddch(body[0].y, body[0].x, '#'); // Print snake's new body part attached to its head
		refresh();

		wait_for_milliseconds(200 - (int) (100.0 * number_of_eaten_apple / NUMBER_OF_APPLE)); // Adjust movement speed based on the number of eaten apples
		mvaddch(body[body_length - 1].y, body[body_length - 1].x, ' '); // Remove snake's body part at the end of its body
		refresh();

		/* Update body position data */
		shift_right_array(body, body_length);
		pos_t temp_pos = { next_y, next_x };
		body[0] = temp_pos; // (pos_t) { next_y, next_x }; (M$ Visual Studio sucks that even it doesn't understand this standard C/C++ code. That's why I put temp_pos.)

		/* Change direction of movement by checking keyboard input */
		if (kbhit())
		{
			int target_direction;
			switch (getch())
			{
				case 'a':
				case KEY_LEFT:
					target_direction = (int) next_direction - 1;
					break;
				case 'd':
				case KEY_RIGHT:
					target_direction = (int) next_direction + 1;
					break;
			}

			if (target_direction < (int) UP)
				next_direction = LEFT;
			else if (target_direction > (int) LEFT)
				next_direction = UP;
			else
				next_direction = (DIRECTION) target_direction;
		}

		
		switch (next_direction)
		{
			case UP: next_y--; break;
			case DOWN: next_y++; break;
			case RIGHT: next_x++; break;
			case LEFT: next_x--; break;
		}

		/* Detect a character at the next position */
		chtype char_at_next_position = mvinch(next_y, next_x) &~ A_COLOR;
		if (char_at_next_position == 'N' || char_at_next_position == '#') // Be bumped against one of the borders or snake's body itself
		{
			result = LOSE;
			break;
		}
		else if (char_at_next_position == 'A' && body_length < MAX_BODY_LENGTH) // Eat an apple
		{
			body_length++;
			number_of_eaten_apple++;

			if (number_of_eaten_apple >= NUMBER_OF_APPLE)
			{
				result = WIN;
				break;
			}
		}
	}

	/* Show a message depending on the game result */
	clear();
	char ending_text[100];
	char ending_sub_text[] = "Automatically ends in %d seconds";
	if (result == WIN)
		strcpy(ending_text, "YOU WIN! - GAME OVER\0");
	else
		strcpy(ending_text, "YOU LOSE - GAME OVER\0");
	attron(COLOR_PAIR(2));
	mvprintw(center_y - 1, center_x - strlen(ending_text)/2, ending_text);
	attroff(COLOR_PAIR(2));
	mvprintw(center_y + 1, center_x - strlen(ending_sub_text)/2, ending_sub_text, 3);
	refresh();
	wait_for_milliseconds(1000);
	mvprintw(center_y + 1, center_x - strlen(ending_sub_text) / 2, ending_sub_text, 2);
	refresh();
	wait_for_milliseconds(1000);
	mvprintw(center_y + 1, center_x - strlen(ending_sub_text) / 2, ending_sub_text, 1);
	refresh();
	wait_for_milliseconds(1000);
	mvprintw(center_y + 1, center_x - strlen(ending_sub_text) / 2, ending_sub_text, 0);
	refresh();

	/* Release the screen resources and quit game */
	endwin();
	return 0;
}

int kbhit(void)
{
	int ch = getch();

	if (ch != ERR)
	{
		ungetch(ch);
		return 1;
	}
	else
	{
		return 0;
	}
}

void wait_for_milliseconds(int milliseconds)
{
	#ifdef _WIN64
	    Sleep(milliseconds);
	#elif _WIN32
	    Sleep(milliseconds);
	#elif __APPLE__
	    usleep(1000 * milliseconds);
	    #if TARGET_IPHONE_SIMULATOR
	    #elif TARGET_OS_IPHONE
	    #elif TARGET_OS_MAC
	    #else
	    #endif
	#elif __linux
	    /* Not yet tested on this platform */
	    usleep(1000 * milliseconds);
	#elif __unix
	    /* Not yet tested on this platform */
	    usleep(1000 * milliseconds);
	#elif __posix
	    /* Not yet tested on this platform */
	    usleep(1000 * milliseconds);
	#endif
}

void shift_right_array(pos_t *array, int array_size) {
	for (int i = array_size - 2; i >= 0; i--)
	{
		*(array + i + 1) = *(array + i);
	}
}