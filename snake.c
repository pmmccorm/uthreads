/******************************************************************************
Snake Game (w/User-level threads)
Copyright (c) 2014, Kendall Stewart

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. 
*******************************************************************************/

#include <stdio.h>		/* printf */
#include <string.h>		/* memset */
#include <unistd.h>		/* close, usleep */
#include <stdlib.h>		/* rand */

#include <sys/ioctl.h>		/* ioctl, TIOCGWINSZ */
#include <fcntl.h>		/* open */

#include <termios.h>		/* tcgetattr, tcsetattr */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <aio.h>

// uthread
#include "uthread.h"
#include "uthread_aio.h"

/* Terminal State
 * see termios(3) for info
 */
struct termios old, new;
void set_term_state()
{
	tcgetattr(0, &old);

	new = old;
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &new);
}

void restore_term_state()
{
	tcsetattr(0, TCSANOW, &old);
}

/* Get screen height
 * see ioctl(2) for info 
 * also: http://rosettacode.org/wiki/Terminal_control/Dimensions#C
 */
int screen_height;
void set_screensize()
{
	struct winsize ws;

	int term = open("/dev/tty", O_RDWR);
	ioctl(term, TIOCGWINSZ, &ws);
	close(term);

	screen_height = ws.ws_row;
}

/* Game State */

#define WIDTH 50
#define HEIGHT 25

char board[HEIGHT][WIDTH];

typedef struct {
	int x;
	int y;
} point;

#define MAX_SNAKE_LENGTH 1250
point snake[MAX_SNAKE_LENGTH];

int snake_head = 0;
int snake_tail = 0;

void set_point(point p)
{
	board[p.y][p.x] = '@';
}

void unset_point(point p)
{
	board[p.y][p.x] = ' ';
}

int game_over = 0;

void initialize_board()
{
	memset(board, ' ', HEIGHT * WIDTH);
	memset(snake, 0, MAX_SNAKE_LENGTH * sizeof(point));

	point initial_point = { WIDTH / 2, HEIGHT / 2 };
	snake[0] = initial_point;
	set_point(snake[0]);
}

void print_board(void)
{
	int i, j;

	for (i = 0; i < screen_height - (HEIGHT + 2); ++i) {
		printf("\n");
	}

	for (i = 0; i < WIDTH + 2; ++i) {
		printf("*");
	}
	printf("\n");

	for (i = 0; i < HEIGHT; ++i) {
		printf("*");
		for (j = 0; j < WIDTH; ++j) {
			printf("%c", board[i][j]);
		}
		printf("*\n");
	}

	for (i = 0; i < WIDTH + 2; ++i) {
		printf("*");
	}
	printf("\n");

}

typedef enum {
	LEFT,
	UP,
	DOWN,
	RIGHT
} direction;

direction current_direction = UP;

/* Manipulate Game State */
void move_snake()
{

	point head = snake[snake_head];

	int r;
	switch (current_direction) {
	case UP:
		head.y = (r = head.y - 1) < 0 ? HEIGHT - 1 : r;
		break;

	case DOWN:
		head.y = (head.y + 1) % HEIGHT;
		break;

	case LEFT:
		head.x = (r = head.x - 1) < 0 ? WIDTH - 1 : r;
		break;

	case RIGHT:
		head.x = (head.x + 1) % WIDTH;
		break;
	}

	snake_head = (snake_head + 1) % MAX_SNAKE_LENGTH;
	snake[snake_head] = head;

	if (board[head.y][head.x] != '#') {
		unset_point(snake[snake_tail]);
		snake_tail = (snake_tail + 1) % MAX_SNAKE_LENGTH;
	}

	if (board[head.y][head.x] == '@') {
		board[head.y][head.x] = 'x';
		game_over = 1;
	}

	else {
		set_point(snake[snake_head]);
	}
}

/* Keypress Listener */
int waiting_for_key;
int listen_for_keypress(void)
{
	waiting_for_key = 1;
	char buf[3];
	async_read(0, buf, 3);
	current_direction = buf[0] & buf[1] & buf[2];
	waiting_for_key = 0;
	return 0;
}

/* Generate random #s (snake food) */
void generate_bites(void)
{
	unsigned int biteX = rand() % WIDTH;
	unsigned int biteY = rand() % HEIGHT;
	board[biteY][biteX] = '#';

	/* generate a bite every 50 game cycles */
	int i;
	for (i = 0; i < 50; ++i) {
		uthread_yield();
	}
}

/* Slows game rate down, makes it playable */
void delay()
{
	usleep(100000);
}

/* continuously perform a game function
   until the game is over */
int game_loop(void *arg)
{
	void (*func) () = arg;

	while (!game_over) {
		func();
		uthread_yield();
	}

	return 0;
}

/* play the game! */
int begin_game(void *arg)
{
	set_term_state();

	set_screensize();
	initialize_board();

	struct uthread_attr a = UT_DEF_ATTR;

	uthread_create(a, game_loop, listen_for_keypress);
	uthread_create(a, game_loop, generate_bites);
	uthread_create(a, game_loop, print_board);
	uthread_create(a, game_loop, move_snake);
	uthread_create(a, game_loop, delay);

	/* wait until game is over */
	while (!game_over) {
		uthread_yield();
	}

	print_board();
	printf("game over! your score: %d\n"
	       "press any key to exit\n", snake_head - snake_tail);

	/* might already be waiting for a keypress.. if not, wait now */
	if (!waiting_for_key) {
		uthread_create(a, listen_for_keypress, 0);
	}

	while (waiting_for_key) {
		uthread_yield();
	}

	restore_term_state();

	return 0;
}

/* Main */
int main(void)
{
	uthread_create(UT_DEF_ATTR, begin_game, 0);

	return uthread_joinall();
}
