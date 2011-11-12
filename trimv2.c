//ß
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define FLUSH fflush(stdout)
#define FALSE 0
#define TRUE 1

#define SLEEP_TIME 0.9 * 100000000

enum direction {NORTH, EAST, SOUTH, WEST};
enum colours {BLACK, RED, GREEN, YELLOW, BLUE, CYAN, PURPLE, WHITE};
static const char FG[8][8] = {"\e[0;30m","\e[0;31m","\e[0;32m","\e[0;33m","\e[0;34m","\e[0;36m","\e[0;35m","\e[0;37m"};
static const char BG[8][7] = {"\e[40m","\e[41m","\e[42m","\e[43m","\e[44m","\e[46m","\e[45m","\e[48m"};
#define CLEAR_SCREEN fputs("\e[2J",stdout)
#define MOVE_CURSOR(r,c) printf("\e[%d;%dH", r+2, c+1)
#define HIDE_CURSOR fputs("\e[?25l",stdout)
#define SHOW_CURSOR fputs("\e[?25h",stdout)
#define SAVE_CURSOR fputs("\e[s",stdout)
#define RESTORE_CURSOR fputs("\e[u",stdout)
#define SET_BG_COLOR(c) fputs(BG[c],stdout)
#define SET_FG_COLOR(c) fputs(FG[c],stdout)
#define ENTER_LINE_MODE fputs("\e[0",stdout)
#define LEAVE_LINE_MODE fputs("\e[B",stdout)
#define DIAMOND_SYMBOL 96

int key;
int key_received;
int key_wasd_received;
int key_pressed[6];
#define NUM_KEYS 6
#define KEY_WASD 0
#define KEY_Q 1
#define KEY_P 2
#define KEY_ENTER 3
#define KEY_AI_1 4
#define KEY_AI_2 5
#define MAX_FOOD_VALUE 5

void init_term();
void keyboard_handler(int signum);
void cleanup_handler(int signum);

//SNAKE CODE
#define ROWS 22
#define COLS 80

#define INIT_LEN 2
#define MAX_LEN 50
#define INIT_X 40
#define INIT_Y 11

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

#define SNAKE1_HEAD 'A'
#define SNAKE1_BODY 'a'
#define EMPTY_TILE ' '
#define OBSTACLE '%'
#define FOOD '$'

#define OPTIMIZE
#define CHECK_WINSIZE
struct winsize w;

struct snake_segment
{
	int dir;
	int x;
	int y;
	char chr;
};
typedef struct snake_segment snake_segment;
snake_segment copy_seg(snake_segment* segment)
{
	snake_segment s;
	s.x = segment->x;
	s.y = segment->y;
	s.dir = segment->dir;
	s.chr = segment->chr;
	return s;
}

char board[ROWS][COLS];
snake_segment* snake;
int X_COORD;
int Y_COORD;
int length;
int global_food_value;

int p1_score;
int p2_score;

int gameover;
int paused;

void init_snake(FILE* file);
void display();
void move_snake();
void make_food();

void print_scores();
void print_status(const char* status, int len);

void print_green(char c) { printf("\033[1;32m%c\033[0m", c); }
void print_red(char c) { printf("\033[1;31m%c\033[0m", c); }
void print_beige(char c) { printf("\033[0;33m%c\033[0m", c); }

int main (int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Incorrect usage, must indicate level config file!\n");
		printf("Usage: ./snake <level>.cfg [optional random seed]\n");
		return 0;
	}
	
	FILE* flevel = fopen(argv[1], "r");
	if (flevel == NULL)
	{
		printf("Error: unable to open %s\n", argv[1]);
		return 0;
	}
	
	init_snake(flevel);
	//display();
	return 0;
	
	
	//Check the window size:
	#ifdef CHECK_WINSIZE
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		if (w.ws_row < 26)
		{
			printf("Window size too short!\n");
			return 0;
		}
		else if (w.ws_col < 80)
		{
			printf("Window size too narrow!\n");
			return 0;
		}
	#endif
	
	srand(time(NULL));
	init_term();

	MOVE_CURSOR(0,0);
	display();
	
	struct timespec prev_time;
	struct timespec current_time;
	struct timespec sleep_time;
	
	while(!gameover)
	{
		long time_delta;
		clock_gettime(CLOCK_MONOTONIC, &prev_time);
		
		if(key_received) 
		{
			if(key_pressed[KEY_Q])
			{
				key_pressed[KEY_Q] = FALSE;
				FLUSH;
				cleanup_handler(0);
				return 0;
			}
			if (key_pressed[KEY_P])
			{
				while (paused);
			}
			key_received = FALSE;
		}
		
		move_snake();
		
		FLUSH;

		clock_gettime(CLOCK_MONOTONIC, &current_time);
		time_delta = (current_time.tv_sec - prev_time.tv_sec) * 1000000000 + (current_time.tv_nsec - prev_time.tv_nsec);

		sleep_time.tv_nsec = SLEEP_TIME;
		sleep_time.tv_sec = 0;
		while(nanosleep(&sleep_time, &sleep_time) == -1);
	}
	cleanup_handler(0);
	//printf("Thanks for playing!\n");
	return 0;
}

void init_snake(FILE* file)
{
	char c;
	length = 0;
	p1_score = p2_score = 0;
	global_food_value = 1;
	
	snake_segment* temp_snake = (snake_segment*)malloc(sizeof(snake_segment) * 7);
	snake = (snake_segment*)malloc(sizeof(snake_segment) * MAX_LEN);
	
	if (file == NULL)
	{
		X_COORD = INIT_X;
		Y_COORD = INIT_Y;
		length = INIT_LEN;
	
		int i;
		int j;
		
		for (i = 0; i < ROWS; i++)
		{
			for (j = 0; j < COLS; j++)
				board[i][j] = EMPTY_TILE;
		}
		
		board[INIT_Y][INIT_X] = SNAKE1_HEAD;
		snake[0].x = INIT_X;
		snake[0].y = INIT_Y;
		snake[0].dir = RIGHT;
		//fprintf(stderr, "snake[%d]'s pos set to  (%d,%d)\n", 0, snake[0].x, snake[0].y);
		for (i = 1; i < INIT_LEN; i++)
		{
			board[INIT_Y][INIT_X - i] = SNAKE1_BODY;
			snake[i].x = INIT_X - i;
			snake[i].y = INIT_Y;
			snake[i].dir = RIGHT;
			//fprintf(stderr, "snake[%d]'s pos set to  (%d,%d)\n", i, snake[i].x, snake[i].y);
		}
	}
	else
	{
		int s1_seg = 0;
		
		int fx = 0;
		int fy = 0;
		
		while ((c = getc(file)) != EOF)
		{
			switch (c)
			{
				case '0':
					board[fy][fx] = EMPTY_TILE;
					break;
				case '1':
					board[fy][fx] = OBSTACLE;
					break;
				case 'A':
					board[fy][fx] = SNAKE1_HEAD;
					temp_snake[length].x = fx;
					temp_snake[length].y = fy;
					temp_snake[length].chr = 'A';
					length++;
					break;
				case 'a':
					board[fy][fx] = SNAKE1_BODY;
					temp_snake[length].x = fx;
					temp_snake[length].y = fy;
					temp_snake[length].chr = 'a';
					length++;
					break;
				case '\n':
					fy++;
					fx = -1;
					break;
				default:
					printf("Invaild character in config file!");
			}
			fx++;
		}
		
		/*
		 * At this point board[][] is setup and ready to be printed
		 * with display(). But the snake is still "unorganized",
		 * ie, the tail could've been read first etc...
		 * So need to re-order the indices and give each segment a direction!...
		 */
		
		
	}
	//make_food();
}

void reorder_snake(snake_segment** snake, int len)
{
	snake_segment* new_snake = (snake_segment*)malloc(sizeof(snake_segment) * len);
	//memcpy(snake_copy, *snake, sizeof(snake_segment) * len);
	
	int i;
	for (i = 0; i < 0; i++)
	{
		if (*snake[i].chr == 'A')
			
	}
}

void display()
{
	print_scores();
	//Start brown:
	printf("\033[0;33m");
	
	int i;
	int j;
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			if (board[i][j] == FOOD)
			{
				printf("\033[0m"); //End brown
				print_red(FOOD);
				printf("\033[0;33m"); //Restart brown
			}
			else if (board[i][j] == OBSTACLE)
				printf("%c", OBSTACLE);
			else
				printf("%c", board[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	
	//End brown
	printf("\033[0m");
}

void print_scores()
{
	SET_FG_COLOR(WHITE);
	SET_BG_COLOR(BLUE);
	MOVE_CURSOR(0, 0);
	//LEAVE_LINE_MODE;
	printf("Player 1: %2d                                                         Player2: %2d\n", p1_score, p2_score);
	MOVE_CURSOR(ROWS+1,0);
	printf("                                                                                \n");
	MOVE_CURSOR(1, 0);
	/*ENTER_LINE_MODE;
	SET_BG_COLOR(WHITE);
	SET_FG_COLOR(BLACK);*/
}

void print_status(const char* status, int len)
{
	MOVE_CURSOR(ROWS+1, 0);
	printf("%s", status);
	int i;
	for (i = 0; i < 80 - len; i++)
		printf(" ");
	printf("\n");
	MOVE_CURSOR(1, 0);
}

void move_snake()
{
	/* Step for moving the snake
	 * 1. Update the directions for all the segments
	 * 2. Move the snake head forward.
	 * 3. Place an SNAKE1_BODY where the head was.
	 * 4. Place an empty tile where the tail is.
	 * NOTE: the board array must be updated at each step as well
	 */
	
	
	snake_segment temp_head = copy_seg(&snake[0]);
	snake_segment temp_tail = copy_seg(&snake[length - 1]);
	
	//Update each segments direction and position
	snake_segment t1 = snake[0];
	snake_segment t2;
	int i;
	for (i = 1; i < length; i++)
	{
		t2 = snake[i];
		//fprintf(stderr, "Changing snake[%d]'s pos from %d (%d,%d) to snake[%d]'s position: %d(%d,%d)\n", i, snake[i].dir, snake[i].x, snake[i].y, i - 1, t1.dir, t1.x, t1.y);
		snake[i] = t1;
		t1 = t2;
	}
	
	//Change the head's direction and find it's new position
	switch(snake[0].dir)
	{
		case UP:
			if (snake[0].y == 0)
				snake[0].y = ROWS - 1;
			else
				snake[0].y--;
			
			snake[0].dir = UP;
			break;
		case DOWN:
			if (snake[0].y == ROWS - 1)
				snake[0].y = 0;
			else
				snake[0].y++;
			
			snake[0].dir = DOWN;
			break;
		case LEFT:
			if (snake[0].x == 0)
				snake[0].x = COLS - 1;
			else
				snake[0].x--;
			
			snake[0].dir = LEFT;
			break;
		case RIGHT:
			if (snake[0].x == COLS - 1)
				snake[0].x = 0;
			else
				snake[0].x++;
			
			snake[0].dir = RIGHT;
			break;
		default:
			return;
	}
	
	//Collision and food detection
	if (board[snake[0].y][snake[0].x] == SNAKE1_BODY)
	{
		print_status((char*)"Game over!", strlen((char*)"Game over!"));
		gameover = TRUE;
		return;
	}
	else if (board[snake[0].y][snake[0].x] == FOOD)
	{
		length += global_food_value;
		p1_score += global_food_value;
		make_food();
		print_scores();
		
		char msg[21];
		sprintf(msg, "Ate food with value %d", global_food_value);
		print_status(msg, sizeof(msg)/sizeof(char));
	}
	
	//Update the board at the head, head-1 and tail
	board[snake[0].y + 1][snake[0].x] = SNAKE1_HEAD;
	board[temp_head.y + 1][temp_head.x] = SNAKE1_BODY;
	board[temp_tail.y + 1][temp_tail.x] = EMPTY_TILE;
	
	//Print the head's character on the terminal
	MOVE_CURSOR(snake[0].y + 1, snake[0].x);
	print_green(SNAKE1_HEAD);
	
	//Print the head-1 character
	MOVE_CURSOR(temp_head.y + 1, temp_head.x);
	print_green(SNAKE1_BODY);
	
	//Print an empty tile where the tail was
	MOVE_CURSOR(temp_tail.y + 1, temp_tail.x);
	print_beige(EMPTY_TILE);
}

void make_food()
{
	int x = 0;
	int y = 0;
	
	do
	{
		x = rand() % COLS;
		y = rand() % ROWS;
	} while (board[y][x] == SNAKE1_HEAD || board[y][x] == SNAKE1_BODY || board[y][x] == OBSTACLE);
	
	global_food_value = rand() % MAX_FOOD_VALUE + 1;
	
	board[y][x] = FOOD;
	MOVE_CURSOR(y + 1, x);
	//fprintf(stderr, "Cursor moved to (%d, %d)\n", y + 1, x);
	switch (global_food_value)
	{
		case 1:
			printf("\033[1;31m@\033[0m"); //red
			break;
		case 2:
			printf("\033[1;35m@\033[0m"); //purple
			break;
		case 3:
			printf("\033[1;34m@\033[0m"); //blue
			break;
		case 4:
			printf("\033[1;36m@\033[0m"); //cyan
			break;
		case 5:
			printf("\033[1;34m@\033[0m"); //green
			break;
		default:
			printf("\033[1;31m@\033[0m"); //red
	}

	return;
}

void init_term()
{
	gameover = FALSE;
	paused = FALSE;
	CLEAR_SCREEN;
	SET_BG_COLOR(WHITE);
	SET_FG_COLOR(BLACK);
	system("stty -echo -icanon");
	ENTER_LINE_MODE;
	HIDE_CURSOR;
	fcntl(STDIN_FILENO, F_SETFL, O_ASYNC | O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETOWN, getpid());
	setbuf(stdin, NULL);
	setvbuf(stdout, NULL, _IOFBF, 1024);
	signal(SIGIO, keyboard_handler);
	signal(SIGTERM, cleanup_handler);
	signal(SIGINT, cleanup_handler);
	key_received = FALSE;
	key_wasd_received = FALSE;
}

void keyboard_handler(int signum)
{
	key = getchar();
	key_received = TRUE;

	//Buffer latest direction and control keys
	switch (key) 
	{
		case 'w':
			key_pressed[KEY_WASD] = NORTH;
			key_wasd_received = TRUE;
			if (snake[0].dir != DOWN)
				snake[0].dir = UP;
			break;
		case 's':
			key_pressed[KEY_WASD] = SOUTH;
			key_wasd_received = TRUE;
			if (snake[0].dir != UP)
				snake[0].dir = DOWN;
			break;
		case SNAKE1_BODY:
			key_pressed[KEY_WASD] = WEST;
			key_wasd_received = TRUE;
			if (snake[0].dir != RIGHT)		
				snake[0].dir = LEFT;
			break;
		case 'd':
			key_pressed[KEY_WASD] = EAST;
			key_wasd_received = TRUE;
			if (snake[0].dir != LEFT)
				snake[0].dir = RIGHT;
			break;
		case 'q':
		case 'Q':
			key_pressed[KEY_Q] = TRUE;
			break;
		case 'p':
		case 'P':
			key_pressed[KEY_P] = TRUE;
			paused = paused ? FALSE : TRUE; //toggle pause boolean
			break;
		case '\n':
			key_pressed[KEY_ENTER] = TRUE;
			break;
		case '1':
			key_pressed[KEY_AI_1] = TRUE;
			break;
		case '2':
			key_pressed[KEY_AI_2] = TRUE;
			break;
		default:
			break;
	}
}

void cleanup_handler(int signum)
{
	int row = 0;
	SET_BG_COLOR(WHITE);
	SET_FG_COLOR(BLACK);
	/*Exit line art mode*/
	LEAVE_LINE_MODE;
	MOVE_CURSOR(row, 0);
	SHOW_CURSOR;

	rewind(stdout);
	ftruncate(1,0);

	MOVE_CURSOR(ROWS + 2,0);
	system("stty echo icanon");
	LEAVE_LINE_MODE;

	exit(0);
}

