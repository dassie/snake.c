#define TRUE 1
#define FALSE 0

struct tile
{
	int x;
	int y;
	int chr;
};
typedef struct tile tile;


#define SEEKING 1
#define AVOIDING 2
int food_ahead;

//maximum distance used in Dijikstra's algorithm
#define MAX_DISTANCE 5000

//this array represents the visited list form DAlgo, zero values means the cell is not visited, 1 means visited
char visited[ROWS][COLS];

//the distance calculated by the Dalgo, if visited is not set for a cell, the distance is an intermediate distance that can change
int distance[ROWS][COLS];


//used in the array below
struct Node{
	int x,y,val;
};

//This structure is used to hold a subset of the unvisited set. (the ones which have values that are less than the max distance)
struct Node toVisit[ROWS*COLS];

//number of entries stored in the toVisit array
int toVisitSize = 0;

//insert a node into the toVisit list
void insert(struct Node n){
	int found = 0;
	
	int i;
	for(i=0;i<toVisitSize;i++){
		//if the x,y coordinates are already in the list
		if(toVisit[i].x==n.x && toVisit[i].y==n.y){
			//if the new path is shorter, update the distance value inside the list
			if(n.val<toVisit[i].val)	toVisit[i].val = n.val;
			found = 1;
			continue;
		}
	}
	
	//if the x,y coordinates were not in the list
	if(found ==0){
		//add the node to the end of the list
		toVisit[toVisitSize]=n;
		toVisitSize++;
	}
}

//find the entried in the unvisited list with the smallest distance, remove it from the list and return it
struct Node extractMin(){
	int i,mIndex;
	int min = MAX_DISTANCE;
	
	// if the list is empty, return a node with value -1 (this means the algorithm needs to terminate
	if(toVisitSize==0){
		printf("Queue is empty");
		struct Node t;
		t.val=-1;
		return t;
	}

	//loop through the list and find the position of the element with the smallest distance. and the distance value.
	for(i=0;i<toVisitSize;i++){
		if(toVisit[i].val<min){
			min = toVisit[i].val;
			mIndex = i;
		}
	}
	
	//create a structure to return
	struct Node t = toVisit[mIndex];

	//replace the entry with the last entry in the list (ie. remove it)
	toVisit[mIndex] = toVisit[toVisitSize-1];
	toVisitSize--;
	
	return t;

}

//Prints the unvisited list for testing purposes
void printQueue(){
	int i;
	printf("There are %d nodes\n",toVisitSize);
	for (i==0;i<toVisitSize;i++){
		printf("(%i,  %i,   %i)\n",toVisit[i].val,toVisit[i].x,toVisit[i].y);
	}

}

//shortest path algorithm
int shortest_path(int player){

	//zero out The visited values and set all distance to max_distance
	int r,c;
	for(r=0;r<ROWS;r++){
		for(c=0;c<COLS;c++){
			distance[r][c] = MAX_DISTANCE;
			visited[r][c] = 0;
		}
	}
		
	struct Node currNode,neighbour;
	
	//make a Node with distance 0, and location of snake head and insert it into the unvisited list to get the algorithm started
	currNode.val = 0;
	if (player == P1)
	{
		currNode.x = snake_1[0].x;
		currNode.y = snake_1[0].y;
	}
	else
	{
		currNode.x = snake_2[0].x;
		currNode.y = snake_2[0].y;	
	}
	distance[currNode.y][currNode.x] = 0;
	insert(currNode);
		
		
	//loop until all distance are calculated (no more unvisited cells)
	while(toVisitSize>0){
		
		//get the unvisited node with the smallest distance and mark it as visited
		currNode = extractMin();
		visited[currNode.y][currNode.x]=1;
		
		int x = currNode.x;
		int y = currNode.y;
		
		//x and y coordinates of the neighbour being considered
		int nx,ny;

		//if the up neighbour is not visited and the current distance can be shortened with this new path
		nx = x;
		ny = (y == 0 ? ROWS - 1 : y - 1);
		if((board[ny][nx]==EMPTY_TILE || board[ny][nx]==FOOD) && visited[ny][nx]==0 && distance[ny][nx]>currNode.val+1){
			//update the distance
			distance[ny][nx] = currNode.val+1;

			//insert the neighbour into the unvisited list
			neighbour.val = currNode.val+1;
			neighbour.x = nx;
			neighbour.y = ny;
			insert(neighbour);		
		}
		
		//if the right neighbour is not visited
		nx = (x == COLS - 1 ? 0 : x + 1);
		ny = y;
		if((board[ny][nx]==EMPTY_TILE || board[ny][nx]==FOOD) && visited[ny][nx]==0 && distance[ny][nx]>currNode.val+1){
			distance[ny][nx] = currNode.val+1;
			neighbour.val = currNode.val+1;
			neighbour.x = nx;
			neighbour.y = ny;
			insert(neighbour);		
		}
		
		//if the down neighbour is not visited
		nx = x;
		ny = (y == ROWS - 1 ? 0 : y + 1);
		if((board[ny][nx]==EMPTY_TILE || board[ny][nx]==FOOD) && visited[ny][nx]==0 && distance[ny][nx]>currNode.val+1){
			distance[ny][nx] = currNode.val+1;
			neighbour.val = currNode.val+1;
			neighbour.x = nx;
			neighbour.y = ny;
			insert(neighbour);		
		}
		
		//if the left neighbour is not visited
		nx = (x == 0 ? COLS - 1 : x - 1);
		ny = y;
		if((board[ny][nx]==EMPTY_TILE || board[ny][nx]==FOOD) && visited[ny][nx]==0 && distance[ny][nx]>currNode.val+1){
			distance[ny][nx] = currNode.val+1;
			neighbour.val = currNode.val+1;
			neighbour.x = nx;
			neighbour.y = ny;
			insert(neighbour);		
		}
			
	}

	//at this point the shortest distance from the snake head to every location on the grid is known

	//now reverse engineer the path, starting from the food and going down to the snake head
	int nx,ny;
	
//current x and y values are set to the food position
	int cx = global_food_x;
	int cy = global_food_y;
	
	//distance to the snake head from the current position  (cx,cy)
	int d;
	d = distance[cy][cx];
	
	
	//What to do when the food is not reachable
	if(d==MAX_DISTANCE){

		//printf("no solution\n");
		//exit(0);
		//return switch_back();
		return turn_right();
	}
	
	//if there exists a path from the head to the food, trace the path back until it gets to the cell right beside the snake head
	while(d>1){
		//look in the neighbourhood of the current cell until you find a distance value that is one less than the current value
		//above
		if(distance[(cy == 0 ? ROWS - 1 : cy - 1)][cx]==d-1){
			cy = (cy == 0 ? ROWS - 1 : cy - 1);
		}
		//below
		else if(distance[(cy == ROWS - 1 ? 0 : cy + 1)][cx]==d-1){
			cy = (cy == ROWS - 1 ? 0 : cy + 1);
		}
		//right
		else if(distance[cy][(cx == COLS - 1 ? 0 : cx + 1)]==d-1){
			cx = (cx == COLS - 1 ? 0 : cx + 1);
		}
		else if(distance[cy][(cx == 0 ? COLS - 1 : cx - 1)]==d-1){
			cx = (cx == 0 ? COLS - 1 : cx - 1);
		}
	
		//update the distance
		d = distance[cy][cx];
	}
	
	//now we can figure out the snake direction based on which side of the head we ended up on
	if (player == P1)
	{
		if(cx>snake_1[0].x) return RIGHT;
		if(cx<snake_1[0].x) return LEFT;
		if(cy>snake_1[0].y) return DOWN;
		if(cy<snake_1[0].y) return UP;
	}
	else
	{
		if(cx>snake_2[0].x) return RIGHT;
		if(cx<snake_2[0].x) return LEFT;
		if(cy>snake_2[0].y) return DOWN;
		if(cy<snake_2[0].y) return UP;
	}
	
}
tile get_next_tile(int dir, int x, int y)
{
	tile t;
	switch (dir)
	{
		case UP:
			t.y = (y == 0 ? ROWS - 1 : y - 1);
			t.x = x;
			t.chr = board[t.y][t.x];
			return t;
		case DOWN:
			t.y = (y == ROWS - 1 ? 0 : y + 1);
			t.x = x;
			t.chr = board[t.y][t.x];
			return t;
		case LEFT:
			t.y = y;
			t.x = (x == 0 ? COLS - 1 : x - 1);
			t.chr = board[t.y][t.x];
			return t;
		case RIGHT:
			t.y = y;
			t.x = (x == COLS - 1 ? 0 : x + 1);
			t.chr = board[t.y][t.x];
			return t;
	}
}

//Returns TRUE if direction is clear
int scan_direction(int dir, int x, int y, int len)
{
	int return_bool = 0;
	tile next_tile;
	int i;
	for (i = 0; i < len; i++)
	{
		next_tile = get_next_tile(dir, x, y);
		x = next_tile.x;
		y = next_tile.y;
		
		if (next_tile.chr == EMPTY_TILE || next_tile.chr == FOOD)
			return_bool = TRUE;
		else
			return FALSE;
		
		if (i == 1 && next_tile.chr == FOOD)
			food_ahead = TRUE;
	}
	return return_bool;
}

void longest_length(int current_dir, int x, int y, int player)
{
	int next_dir;
	fprintf(stderr, "(%d,%d)\n", x, y);
	if (scan_direction(current_dir, x, y, 2) == FALSE)
	{
		if (scan_direction(LEFT, x, y, 2) == TRUE)
			next_dir = LEFT;
		else if (scan_direction(RIGHT, x, y, 2) == TRUE)
			next_dir = RIGHT;
		else if (scan_direction(UP, x, y, 2) == TRUE)
			next_dir = UP;
		else if (scan_direction(DOWN, x, y, 2) == TRUE)
			next_dir = DOWN;
		else if (scan_direction(LEFT, x, y, 1) == TRUE)
			next_dir = LEFT;
		else if (scan_direction(RIGHT, x, y, 1) == TRUE)
			next_dir = RIGHT;
		else if (scan_direction(UP, x, y, 1) == TRUE)
			next_dir = UP;
		else if (scan_direction(DOWN, x, y, 1) == TRUE)
			next_dir = DOWN;
	}
	else
		next_dir = current_dir;
	
	snake_1[0].dir = (player == P1 ? next_dir : snake_1[0].dir);
	snake_2[0].dir = (player == P2 ? next_dir : snake_2[0].dir);
	/*if (player == P1)
	{
		snake_1[0].dir = next_dir;
		move_snake(&snake_1, &length_1, &p1_score, P1);
		fprintf(stderr, "Moved snake from (%d,%d) to (%d,%d) in direction %d\n", x, y, snake_1[0].x, snake_1[0].y, next_dir);
	}
	else 
	{
		snake_2[0].dir = next_dir;
		move_snake(&snake_2, &length_2, &p2_score, P2);
	}*/
	
	if (player == P1)
	{
		fprintf(stderr, "moving snake 1\n");
		move_snake(&snake_1, &length_1, &p1_score, P1);
	}
	else
		move_snake(&snake_2, &length_2, &p2_score, P2);
	
	//fprintf(stderr, "longest length called\n");
	
	x = (player == P1 ? snake_1[0].x : snake_2[0].x);
	y = (player == P1 ? snake_1[0].y : snake_2[0].y);
	
	fprintf(stderr, "(%d,%d)\n", x, y);
	
	int delta;
	if (global_food_y > y)
	{
		delta = global_food_y - y;
		if (scan_direction(DOWN, x, y, delta) == TRUE)
			next_dir = DOWN;
		else if (scan_direction(UP, x, y, 2) == TRUE)
			next_dir = UP;
	}
	else if (global_food_y < y)
	{
		delta = y - global_food_y;
		if (scan_direction(UP, x, y, delta) == TRUE)
			next_dir = UP;
		else if (scan_direction(DOWN, x, y, 2) == TRUE)
			next_dir = DOWN;
	}
	else
	{
		if (global_food_x > x)
		{
			delta = global_food_x - x;
			if (scan_direction(RIGHT, x, y, delta) == TRUE)
				next_dir = RIGHT;
			else if (scan_direction(LEFT, x, y, 2) == TRUE)
				next_dir = LEFT;
		}
		else if (global_food_x < x)
		{
			delta = x - global_food_x;
			if (scan_direction(LEFT, x, y, delta) == TRUE)
				next_dir = LEFT;
			else if (scan_direction(RIGHT, x, y, 2) == TRUE)
				next_dir = RIGHT;
		}
	}
	
	snake_1[0].dir = (player == P1 ? next_dir : snake_1[0].dir);
	snake_2[0].dir = (player == P2 ? next_dir : snake_2[0].dir);
	
	fprintf(stderr, "Changed direction to %d\n", next_dir);
}

int switch_back(int current_dir, int x, int y, int* obs, int* prev_dir)
{
	char next_tile;
	char lr_tile;
	
	if (*obs == TRUE)
	{
		*obs = FALSE;
		switch (*prev_dir)
		{
			case UP:
				return DOWN;
			case DOWN:
				return UP;
			case LEFT:
				return RIGHT;
			case RIGHT:
				return LEFT;
			default:
				return UP;
		}
	}
	else
	{
		switch (current_dir)
		{
			case UP:
				next_tile = board[(y == 0 ? ROWS - 1 : y - 1)][x];
				if (next_tile != EMPTY_TILE && next_tile != FOOD)
				{
					*obs = TRUE;
					*prev_dir = current_dir;
					
					if (board[y][x + 1] == EMPTY_TILE || board[y][x + 1] == FOOD)
						return RIGHT;
					if (board[y][x - 1] == EMPTY_TILE || board[y][x - 1] == FOOD)
						return LEFT;
					else
						return LEFT;		
				}
				else
					return current_dir;
			case DOWN:
				next_tile = board[(y == ROWS - 1 ? 0 : y + 1)][x];
				if (next_tile != EMPTY_TILE && next_tile != FOOD)
				{
					*obs = TRUE;
					*prev_dir = current_dir;
					
					if (board[y][x + 1] == EMPTY_TILE || board[y][x + 1] == FOOD)
						return RIGHT;
					if (board[y][x - 1] == EMPTY_TILE || board[y][x - 1] == FOOD)
						return LEFT;
					else
						return LEFT;
				}
				else
					return current_dir;
			case LEFT:
				next_tile = board[y][(x == 0 ? COLS - 1 : x - 1)];
				if (next_tile != EMPTY_TILE && next_tile != FOOD)
				{
					*obs = TRUE;
					*prev_dir = current_dir;
					if (board[y - 1][x] == EMPTY_TILE || board[y - 1][x] == FOOD)
						return UP;
					else if (board[y + 1][x] == EMPTY_TILE || board[y + 1][x] == FOOD)
						return DOWN;
					else
						return DOWN;
				}
				else
					return current_dir;
			case RIGHT:
				next_tile = board[y][(x == COLS - 1 ? 0 : x + 1)];
				if (next_tile != EMPTY_TILE && next_tile != FOOD)
				{
					*obs = TRUE;
					*prev_dir = current_dir;
					
					if (board[y + 1][x] == EMPTY_TILE || board[y + 1][x] == FOOD)
						return DOWN;
					if (board[y - 1][x] == EMPTY_TILE || board[y - 1][x] == FOOD)
						return UP;
					else
						return DOWN;
				}
				else
					return current_dir;
			default:
				return current_dir;
		}
	}
}

int go_east(int current_dir, int x, int y, int* obs, int* prev_dir)
{
	//Is it possible to go east?
	char east_tile;
	if (x == COLS - 1)
		east_tile = board[y][0];
	else
		east_tile = board[y][x + 1];
	if (east_tile == EMPTY_TILE || east_tile == FOOD)
		return RIGHT;
	
	//East didn't work, so go in the first available direction:
	//UP
	char next_tile;
	if (y == 0)
		next_tile = board[ROWS - 1][x];
	else
		next_tile = board[y - 1][x];
	if (next_tile == EMPTY_TILE || next_tile == FOOD)
		return UP;
	
	//DOWN
	if (y == ROWS - 1)
		next_tile = board[0][x];
	else
		next_tile = board[y + 1][x];
	if (next_tile == EMPTY_TILE || next_tile == FOOD)
		return DOWN;
	
	//LEFT
	if (x == 0)
		next_tile = board[y][COLS - 1];
	else
		next_tile = board[y][x - 1];
	if (next_tile == EMPTY_TILE || next_tile == FOOD)
		return LEFT;
	
	//If all else fails, go right anyway
	return RIGHT;
}

int greedy(int current_dir, int x, int y, int* obs, int* prev_dir)
{
	char next_tile;
	int failed = 0;
	
	if (*obs == TRUE)
	{
		*obs = FALSE;
		switch (*prev_dir)
		{
			case UP:
				return DOWN;
			case DOWN:
				return UP;
			case LEFT:
				return RIGHT;
			case RIGHT:
				return LEFT;
			default:
				return UP;
		}
	}
	
	if (global_food_y < y)
	{
		
		next_tile = board[(y == 0 ? ROWS - 1 : y - 1)][x];;
		if (next_tile != EMPTY_TILE)
			failed++;
			
		if (current_dir == DOWN)
		{
			*obs = TRUE;
			if (board[y][x - 1] == EMPTY_TILE)
			{
				*prev_dir == DOWN;
				return LEFT;
			}
			else if (board[y][x + 1] == EMPTY_TILE)
			{
				*prev_dir = DOWN;
				return RIGHT;
			}
		}
		else
			return UP;
	}
	else if (global_food_y > y)
	{
		next_tile = board[(y == ROWS - 1 ? 0 : y + 1)][x];
		if (next_tile != EMPTY_TILE)
			failed++;
			
		if (current_dir == UP)
		{
			*obs = TRUE;
			if (board[y][x - 1] == EMPTY_TILE)
			{
				*prev_dir == UP;
				return LEFT;
			}
			else if (board[y][x + 1] == EMPTY_TILE)
			{
				*prev_dir = UP;
				return RIGHT;
			}
		}
		else
			return DOWN;
	}
	
	if (global_food_x > x)
	{
		next_tile = board[y][(x == COLS - 1 ? 0 : x + 1)];
		if (next_tile != EMPTY_TILE && failed >= 1)
			return turn_right(current_dir, x, y, obs, prev_dir);
			
		if (current_dir == LEFT)
		{
			*obs = TRUE;
			if (board[y - 1][x] == EMPTY_TILE)
			{
				*prev_dir = LEFT;
				return UP;
			}
			else if (board[y + 1][x] == EMPTY_TILE)
			{
				*prev_dir = LEFT;
				return DOWN;
			}
		}
		else
			return RIGHT;
	}
	else if (global_food_x < x)
	{
		next_tile = board[y][(x == 0 ? COLS - 1 : x + 1)];
		if (next_tile != EMPTY_TILE && failed >= 1)
				return turn_right(current_dir, x, y, obs, prev_dir);
				
		if (current_dir == RIGHT)
		{
			*obs = TRUE;
			if (board[y - 1][x] == EMPTY_TILE)
			{
				*prev_dir = RIGHT;
				return UP;
			}
			else if (board[y + 1][x] == EMPTY_TILE)
			{
				*prev_dir = RIGHT;
				return DOWN;
			}
		}
		else
			return LEFT;
	}
}

int turn_right(int current_dir, int x, int y, int* obs, int* prev_dir)
{
	char next_tile;
	
	switch(current_dir)
	{
		case UP:
			next_tile = board[(y == 0 ? ROWS - 1 : y - 1)][x];
			if (next_tile == EMPTY_TILE || next_tile == FOOD)
				return UP;
			else
				return RIGHT;
		
		case RIGHT:
			next_tile = board[y][(x == COLS - 1 ? 0 : x + 1)];
			if (next_tile == EMPTY_TILE || next_tile == FOOD)
				return RIGHT;
			else
				return DOWN;
		
		case DOWN:
			next_tile = board[(y == ROWS - 1 ? 0 : y + 1)][x];
			if (next_tile == EMPTY_TILE || next_tile == FOOD)
				return DOWN;
			else 
				return LEFT;
		
		case LEFT:
			next_tile = board[y][(x == 0 ? COLS - 1 : x - 1)];
			if (next_tile == EMPTY_TILE || next_tile == FOOD)
				return LEFT;
			else
				return UP;
	}
}
