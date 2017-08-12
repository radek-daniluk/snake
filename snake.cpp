/* snake.cpp
 * 30.12.2016
 *
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <queue>

void clean( void );
void initNewGame( void );

typedef enum { up, down, left, right } Direction;
typedef enum { empty=0, head, mid, tail, item, wall } Tile; // see board[][]
typedef enum { running, stopped, paused, gameOver } gameState;

void changeDir( Direction );
void findNext( void ); // find next tile of the snake's movement -see dir & pnext
bool movePossible( void ); // check if snake can go forward
void moveSnake( void );

void renderBoard( void );
void printBoardCLI( void );

const int height = 30;
const int width = 40;
const int tile = 16;

const int screenWidth = tile * width;
const int screenHeight = tile * height;

gameState state;

// game board represents current state of the game
Tile board[height][width];

// Snake is a FIFO. Back of the FIFO is snake head, front is snake tail.
std::queue< Tile* > waz;

// Current direction of the snake's movement
Direction dir;

// Direction in the next move
Direction newDir;

// Pointer to the next tile of the snake's movement
Tile * pnext;

SDL_Renderer* sdlRenderer = NULL;
SDL_Window* screen = NULL;

int main(int argc, char** argv) {

	SDL_Init( SDL_INIT_VIDEO );
	if( SDL_CreateWindowAndRenderer(
		   screenWidth, screenHeight,
		   SDL_WINDOW_SHOWN, &screen, &sdlRenderer)
	  )	{
			printf("Window or renderer creation failed - exiting.");
			return 1;
		}

	SDL_SetWindowTitle( screen, "Żmijka" );

	initNewGame();

	SDL_Event event;

	while( state == running ) {

		SDL_Delay( 75 ); /* after stepping if necessary and polling and handling events -> wait */

		while( SDL_PollEvent( &event ) ) {
			switch( event.type ) {
				case SDL_KEYDOWN:
					switch( event.key.keysym.sym ) {
						case SDLK_ESCAPE: /* quit game */
						case SDLK_q:
							state = stopped;
							break;
						case SDLK_p: /* pause game */
							state = paused;
							break;
						case SDLK_LEFT:
							changeDir(left);
							break;
						case SDLK_RIGHT:
							changeDir(right);
							break;
						case SDLK_UP:
							changeDir(up);
							break;
						case SDLK_DOWN:
							changeDir(down);
							break;
						}
						break;
				case SDL_QUIT:
					state = stopped;
					break;
			} /*switch event.type*/
		}/*while pollEvent*/

		dir = newDir;
		findNext();
		if( movePossible() )
			moveSnake();
		else {
			state = gameOver;
		}

		//printBoardCLI();
		renderBoard();

	}/*while running*/

	/* clean before you go out the toilet */
	clean();

	SDL_Quit();

	return 0;
}

void initNewGame( void ) {

	// set the initial pozition of the snake in the game board and
	// in the snake queue
	board[4][3] = tail; // end of the snake
	waz.push( &board[4][3] );

	// middle of the snake (2 tiles)
	board[4][4] = board[4][5] = mid;
	waz.push( &board[4][4] );
	waz.push( &board[4][5] );

	// head of the snake
	board[4][6] = head;
	waz.push( &board[4][6] );

	// initial direction of the snake's movement
	newDir = dir = right;

	// initial items
	board[6][4]=board[2][1]=board[2][10]=board[20][16]=item;

	for(int i=0; i<height; i++)
		board[i][0] = wall;

	board[20][13]=board[20][14]=board[20][15]=board[21][16]=item;

	renderBoard();

	// start the game loop
	state = running;
}

// draw whole board to the screen
void renderBoard ( void ) {

	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = rect.h = tile;

	for(int i=0; i<height; i++) {
		rect.y = i*tile;
		for(int j=0; j<width; j++) {
			rect.x = j*tile;
			switch( board[i][j] ) {
				case empty:SDL_SetRenderDrawColor( sdlRenderer, 0x80, 0x80, 0x80, 0xFF );break;
				case head: SDL_SetRenderDrawColor( sdlRenderer, 0x80, 0xFF, 0x80, 0xFF );break;
				case mid:  SDL_SetRenderDrawColor( sdlRenderer, 0x00, 0xFF, 0x00, 0xFF );break;
				case tail: SDL_SetRenderDrawColor( sdlRenderer, 0xA0, 0xFF, 0xA0, 0xFF );break;
				case wall: SDL_SetRenderDrawColor( sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF );break;
				case item: SDL_SetRenderDrawColor( sdlRenderer, 0xFF, 0x00, 0x00, 0xFF );break;
				default:   SDL_SetRenderDrawColor( sdlRenderer, 0x00, 0x00, 0x00, 0xFF );
			}
			SDL_RenderFillRect( sdlRenderer , &rect );
		}
	}

	SDL_RenderPresent( sdlRenderer );
}

// if it is possible change direction of the sanke's movement
// cannot reverse direction e.g. change from up to down, from left to right etc.
void changeDir( Direction changedDir ) {
	if(dir == up || dir == down) {
		if(changedDir == left || changedDir == right) {
			newDir = changedDir;
		}
	} else {
		if(changedDir == up || changedDir == down) {
			newDir = changedDir;
		}
	}
}

// find the coordinates of the next tile according to snakes dir(ection)
void findNext( void ) {
	int x, y, nx, ny;
	int temp = ( waz.back() - &board[0][0] );
	y = temp/width; // get x and y coordinates of snake's head on the board
	x = temp%width;

	if(dir == right) {
		ny = y;
		if(x == width - 1) { // check right boundary of the game board
			nx = 0;
		} else {
			nx = x+1;
		}
	} else if (dir == left) {
		ny = y;
		if(x == 0) { // check left boundary of the game board
			nx = width - 1;
		} else {
			nx = x - 1;
		}
	} else if (dir == up) {
		nx = x;
		if(y == 0) { // check top boundary of the game board
			ny = height - 1;
		} else {
			ny = y-1;
		}
	} else if (dir == down) {
		nx = x;
		if (y == height - 1) { // check bottom boundary of the game board
			ny = 0;
		} else {
			ny = y+1;
		}
	}

	pnext = &board[ny][nx];
}

// check if the next tile on the game board is empty or
// has an item or is snake's tail
bool movePossible( void ) {
	if( *pnext == empty || *pnext == item || *pnext == tail )
		return true;
	else
		return false;
}

// update game board and snake queue on snake's move
void moveSnake( void ) {

	if( *pnext != item ) {		// if snake do not eat an item
		*(waz.front()) = empty; // game board update
		waz.pop();					// queue update
		*(waz.front()) = tail;	// game board update
	}

	*pnext = head;			// move snake head in game board
	*(waz.back()) = mid; // old head is now mid of snake (board)
	waz.push( pnext );		// move forward snake ( snake queue )
}

void clean( void ) {

	SDL_DestroyRenderer( sdlRenderer );
	SDL_DestroyWindow( screen );
	sdlRenderer = NULL;
	screen = NULL;
}

void printBoardCLI( void ) {

	printf("------------------------\n");
	for(int i=0; i<height; i++) {
		for(int j=0; j<width; j++)
			switch( board[i][j] ) {
				case empty:printf(".");break;
				case head: printf("H");break;
				case mid:  printf("*");break;
				case tail: printf("E");break;
				case wall: printf("#");break;
				case item: printf("$");break;
				default:   printf("!");
			}
		printf("\n");
	}
}

