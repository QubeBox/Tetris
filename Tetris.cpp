#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <GL/glut.h>

using namespace std;

class Square {

public:
	bool isFilled;			// Is the cell filled
	bool isActive;			// Is the cell active
	bool toBeDeleted;		// Should the cell be deleted
	float red, green, blue; // Cell color
};

const int numPieces = 7;	// Number of different pieces
const int numRotations = 4; // Number of turns for each piece
const int numSpaces = 8;	// Memory capacity for storing information about each piece

// Game piece class
class Piece {

public:

	// Constructors
	Piece();
	Piece(int newPiece);

	// Piece coordinates (calculated from the upper left corner of the piece, even if this corner is empty)
	int x;
	int y;

	// Color values
	float redVal, greenVal, blueVal;

	// Piece type and rotation
	int type;
	int rotation;

	// Rotating a piece
	void rotatePiece(int dir);

	// Setting the piece color in RGB format
	void color(float r, float g, float b);

	// Values for rotating pieces
	const int* rotations();
};

// All pieces with each option of their rotation in the grid in the format {x0, y0, x1, y1, x2, y2, x3, y3}
const int gamePieces[numPieces][numRotations][numSpaces] =
{
	{
		{0, 0, 1, 0, 0, 1, 1, 1}, // Square
		{0, 0, 1, 0, 0, 1, 1, 1},
		{0, 0, 1, 0, 0, 1, 1, 1},
		{0, 0, 1, 0, 0, 1, 1, 1},
	},
	{
		{0, 0, 0, 1, 0, 2, 0, 3}, // Vertical line
		{0, 0, 1, 0, 2, 0, 3, 0},
		{0, 0, 0, 1, 0, 2, 0, 3},
		{0, 0, 1, 0, 2, 0, 3, 0},
	},
	{
		{0, 0, 0, 1, 1, 1, 0, 2}, // T piece
		{1, 0, 0, 1, 1, 1, 2, 1},
		{0, 1, 1, 0, 1, 1, 1, 2},
		{0, 0, 1, 0, 2, 0, 1, 1}
	},
	{	{0, 0, 1, 0, 0, 1, 0, 2}, // L piece
		{0, 0, 0, 1, 1, 1, 2, 1},
		{1, 0, 1, 1, 0, 2, 1, 2},
		{0, 0, 1, 0, 2, 0, 2, 1}
	},
	{	{0, 0, 1, 0, 1, 1, 1, 2}, // Reverse L piece
		{0, 0, 1, 0, 2, 0, 0, 1},
		{0, 0, 0, 1, 0, 2, 1, 2},
		{2, 0, 0, 1, 1, 1, 2, 1}
	},
	{	{0, 0, 0, 1, 1, 1, 1, 2}, // Z piece
		{1, 0, 2, 0, 0, 1, 1, 1},
		{0, 0, 0, 1, 1, 1, 1, 2},
		{1, 0, 2, 0, 0, 1, 1, 1}
	},
	{	{1, 0, 0, 1, 1, 1, 0, 2}, // Reverse Z piece
		{0, 0, 1, 0, 1, 1, 2, 1},
		{1, 0, 0, 1, 1, 1, 0, 2},
		{0, 0, 1, 0, 1, 1, 2, 1}
	}
};

const int ROWS = 20;
const int COLS = 10;

// Main game class
class Game {
public:

	// Constructor
	Game();

	// Main game class methods
	bool canRotate(Piece p);
	bool moveCollision(int dir);
	bool rotationCollision();
	void clearMainGrid();
	void clearNextPieceGrid();
	void genNextPiece();
	void restart();
	void move(int dir);
	void rotateShape(int rot);
	void updateActivePiece();
	void updateNextPieceGrid();
	void fixActivePiece();
	void update();
	void updateActiveAfterCollision();
	void checkLine();
	void clearLine();
	//bool gameOver();

	// Game pieces
	Piece activePiece;
	Piece nextPiece;
	Piece activePieceCopy;

	// Current game grid
	Square mainGrid[ROWS][COLS];

	// Grid with the next piece
	Square nextPieceGrid[5][5];

	// Game data
	bool killed;
	bool paused;
	bool deleteLines;
	int linesCleared;
	int shapesCount;
	int timer;
};

Piece::Piece() {
	Piece(0);
}

/*
Creating a new piece, setting its type, color and base rotation
*/
Piece::Piece(int numType) {
	type = numType;
	switch (type) {
	case 0: color(1.0, 1.0, 0.0); break;
	case 1: color(0.5, 0.5, 0.5); break;
	case 2: color(0.0, 1.0, 1.0); break;
	case 3: color(0.0, 0.0, 1.0); break;
	case 4: color(1.0, 0.0, 0.0); break;
	case 5: color(1.0, 0.0, 1.0); break;
	case 6: color(0.0, 0.8, 0.0); break;
	}
	rotation = 0;
}

/*
Getting an array with information about a specific piece based on its rotation
*/
const int* Piece::rotations() {
	return gamePieces[type][rotation];
}

/*
Setting the color of a piece
*/
void Piece::color(float r, float g, float b) {
	redVal = r;
	greenVal = g;
	blueVal = b;
}

/*
Increase or decrease the rotation index of game pieces
*/
void Piece::rotatePiece(int dir) {
	if (dir > 0) {
		if (rotation == 3)
			rotation = 0;
		else
			rotation += dir;
	}
	else {
		if (rotation == 0)
			rotation = 3;
		else
			rotation += dir;
	}
}

void Game::update() {
	// Check pieces collision
	if (moveCollision(0)) {					// If there was a collision
		if (activePiece.y <= 2) {			// checking if it ends the game
			killed = true;
		}
		else {								// If the game is still active
			updateActiveAfterCollision();	// The figure is fixed in place of the collision
			checkLine();					// Checking for filled lines
			if (deleteLines)				// If lines were found to be deleted,
				clearLine();				// filled lines are removed
			genNextPiece();					// Generating a new piece

			clearNextPieceGrid();
			updateNextPieceGrid();

			updateActivePiece();			// Its update in the game grid
		}
	}
	else {									// If there was no collision, the piece drops below
		fixActivePiece();
		activePiece.y++;
		updateActivePiece();
	}
}

/*
Starting a new game and initializing the required elements
*/
void Game::restart()
{
	clearMainGrid();		// Cleaning the main grid
	clearNextPieceGrid();	// Clearing the grid with the next figure
	linesCleared = 0;		// The player's score is zero
	shapesCount = 1;		// The counter of pieces per game is equal to one
	killed = false;
	paused = false;
	deleteLines = false;

	// Generating the current random piece
	activePiece = Piece(rand() % numPieces);
	activePiece.x = COLS / 2;
	activePiece.y = 0;
	updateActivePiece();

	// Generating the next piece
	nextPiece = Piece(rand() % numPieces);
	nextPiece.x = COLS / 2;
	nextPiece.y = 0;
	updateNextPieceGrid();
}

/*
Updating the game grid and the correct display of the active figure when falling
*/
void Game::fixActivePiece() {
	// Determining the data of the current piece by its type and position
	const int* trans = activePiece.rotations();
	for (int i = 0; i < 8; i += 2) {
		Square& square = mainGrid[activePiece.y + trans[i + 1]][activePiece.x + trans[i]];
		// Setting active and inactive cells
		square.isFilled = false;
		square.isActive = false;
	}
}

/*
Create the next game piece
*/
void Game::genNextPiece() {
	activePiece = nextPiece;
	nextPiece = Piece(rand() % numPieces);
	nextPiece.x = COLS / 2;
	nextPiece.y = 0;
	// Increase the piece counter per game
	shapesCount++;
}

/*
Move the active piece left and right
*/
void Game::move(int dir)
{
	if (moveCollision(dir))	// If there is a collision with one of the borders,
		return;				// nothing happens
	fixActivePiece();
	activePiece.x += dir;
	updateActivePiece();
}

/*
Cleaning the main grid
*/
void Game::clearMainGrid()
{
	for (int r = 0; r < ROWS; r++) {
		for (int c = 0; c < COLS; c++) {
			mainGrid[r][c].isFilled = false;
			mainGrid[r][c].isActive = false;
		}
	}
}

/*
Clearing the grid with the next piece
*/
void Game::clearNextPieceGrid()
{
	for (int r = 0; r < 5; r++) {
		for (int c = 0; c < 5; c++) {
			nextPieceGrid[r][c].isFilled = false;
			nextPieceGrid[r][c].isActive = false;
		}
	}
}

/*
Updating the position of the active piece with rotation
*/
void Game::updateActivePiece() {
	// Pointer to an array that stores all conversions
	const int* trans = activePiece.rotations();
	for (int i = 0; i < 8; i += 2) {
		// Find the active piece in the game grid
		Square& square = mainGrid[activePiece.y + trans[i + 1]][activePiece.x + trans[i]];
		// Convert the active piece to filled grid cells
		square.isFilled = true;
		square.isActive = true;
		square.red = activePiece.redVal;
		square.green = activePiece.blueVal;
		square.blue = activePiece.greenVal;
	}
}

/*
Updating the grid with the next piece
*/
void Game::updateNextPieceGrid() {
	// Pointer to an array that stores all conversions
	const int* transNext = nextPiece.rotations();
	for (int i = 0; i < 8; i += 2) {
		// Find the active piece in the game grid
		Square& squareNext = nextPieceGrid[nextPiece.y + transNext[i + 1]][nextPiece.x + transNext[i]];
		// Convert the active piece to filled grid cells
		squareNext.isFilled = true;
		squareNext.isActive = true;
		squareNext.red = nextPiece.redVal;
		squareNext.green = nextPiece.blueVal;
		squareNext.blue = nextPiece.greenVal;
	}
}

/*
Constructor
*/
Game::Game()
{
	restart();
	timer = 500;
}

/*
Rotate the current piece and check if it can be rotated
*/
void Game::rotateShape(int dir) {
	// Create a copy of the active piece and check if it can be rotated
	activePieceCopy = Piece(rand() % numPieces);
	activePieceCopy.x = activePiece.x;
	activePieceCopy.y = activePiece.y;
	activePieceCopy.rotation = activePiece.rotation;
	activePieceCopy.type = activePiece.type;
	activePieceCopy.rotatePiece(dir);

	// If the active piece can be rotated, it is rotated and displayed
	if (canRotate(activePieceCopy)) {
		fixActivePiece();
		activePiece.rotatePiece(dir);
		updateActivePiece();
	}
}

/*
Checking whether a piece can be rotated
*/
bool Game::canRotate(Piece activeP) {
	if (rotationCollision()) {
		return false;
	}
	else
		return true;
}

/*
Checking for collisions when rotating a piece
*/
bool Game::rotationCollision() {
	int x, y;
	const int* trans = activePieceCopy.rotations();
	for (int i = 0; i < 8; i += 2) {
		x = activePieceCopy.x + trans[i];
		y = activePieceCopy.y + trans[i + 1];

		if (x >= COLS || y >= ROWS || x < 0 || (mainGrid[y][x].isFilled && !mainGrid[y][x].isActive))
			return true;
	}
	return false;
}

/*
Checking for collisions when the piece is moving
*/
bool Game::moveCollision(int dir) {
	int x, y;
	const int* trans = activePiece.rotations();
	for (int i = 0; i < 8; i += 2) {
		x = activePiece.x + trans[i];
		y = activePiece.y + trans[i + 1];
		if (dir == 0)
			y += 1;
		else
			x += dir;
		if (x >= COLS || y >= ROWS || x < 0 || (mainGrid[y][x].isFilled && !mainGrid[y][x].isActive))
			return true;
	}
	return false;
}

/*
Updating the location of the active piece after a collision
*/
void Game::updateActiveAfterCollision() {
	const int* trans = activePiece.rotations();
	for (int i = 0; i < 8; i += 2) {
		Square& square = mainGrid[activePiece.y + trans[i + 1]][activePiece.x + trans[i]];
		square.isActive = false;
	}
}

/*
Checking lines for filling and setting filled lines for deleting
*/
void Game::checkLine() {
	int fullRows = 0;
	for (int r = 0; r < ROWS; r++) {
		bool fullRow = false;
		for (int c = 0; c < COLS; c++) {
			Square& square = mainGrid[r][c];
			if (square.isFilled) {
				fullRow = true;
			}
			else {
				fullRow = false;
				break;
			}
		}
		if (fullRow) {
			for (int c = 0; c < COLS; c++) {
				mainGrid[r][c].toBeDeleted = true;
			}
			deleteLines = true;
			linesCleared++;
		}
	}
}

/*
Remove a filled row and move all pieces up one cell down
*/
void Game::clearLine() {
	for (int r = ROWS - 1; r > 0; r--) { // Checking each line
		int linesDeleted = 0;
		if (mainGrid[r][0].toBeDeleted) {
			for (int r2 = r; r2 > 0; r2--) { // Move all rows down one cell
				for (int c = 0; c < COLS; c++) {
					mainGrid[r2][c].isFilled = mainGrid[r2 - 1][c].isFilled;
					mainGrid[r2][c].isActive = mainGrid[r2 - 1][c].isActive;
					mainGrid[r2][c].toBeDeleted = mainGrid[r2 - 1][c].toBeDeleted;
					mainGrid[r2][c].red = mainGrid[r2 - 1][c].red;
					mainGrid[r2][c].green = mainGrid[r2 - 1][c].green;
					mainGrid[r2][c].blue = mainGrid[r2 - 1][c].blue;
				}
			}
			r++;
		}
	}
	deleteLines = false;

}
const int BLOCKSIZE = 40;
const int VPWIDTH = COLS * BLOCKSIZE;
const int VPHEIGHT = ROWS * BLOCKSIZE;

Game game;
GLvoid* font_style = GLUT_BITMAP_TIMES_ROMAN_24;

// Displaying text on the screen
void BitmapText(char* str, int wcx, int wcy)
{
	glRasterPos2i(wcx, wcy);
	for (int i = 0; str[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
	}
}

// Getting a random number in the interval [0, 1]
double random()
{
	int r = rand();
	return double(r) / RAND_MAX;
}

// Callback function, timer
void timer(int id)
{
	if (game.killed) {	// If the game is over

		game.paused = true;
		//glutTimerFunc(200, timer, id);
		game.clearMainGrid();
		game.clearNextPieceGrid();
		glutPostRedisplay();

	}
	else if (!game.paused) {	// If the game continues and is not paused
		game.update();
		if (game.killed) {
			glutTimerFunc(10, timer, 1);
		}
		else {
			glutPostRedisplay();
			glutTimerFunc(game.timer, timer, 0);
		}
	}
}

// Callback function, handling keyboard actions
void keyboard(unsigned char key, int x, int y)
{
	if (game.paused && game.killed) {
		if (key == 13) { // 13 == ENTER
			game.killed = false;
			game.restart();
			glutTimerFunc(game.timer, timer, 0);
		}
	}
	else {
		if (key == 'p' || key == 27) { // 27 == ESCAPE
			game.paused = !game.paused;
			if (!game.paused)
				glutTimerFunc(game.timer, timer, 0);
		}
		else if (!game.paused && !game.killed && key == ' ') { // ' ' == SPACE
			game.update();
		}
	}

	glutPostRedisplay();
}

// Callback function, handling arrow actions
void special(int key, int x, int y)
{
	if (!game.paused && !game.killed) {
		if (key == GLUT_KEY_LEFT) {
			game.move(-1);
			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_RIGHT) {
			game.move(1);
			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_UP) {
			game.rotateShape(1);
			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_DOWN) {
			game.rotateShape(-1);
			glutPostRedisplay();
		}
	}
}

// Callback function, displaying the gameplay window
void display(void)
{
	const int N = 100;
	char msg[N + 1];

	glClearColor(0.2f, 0.2f, 0.2f, 0.72);
	glClear(GL_COLOR_BUFFER_BIT);

	// Grid
	glViewport(0, 0, VPWIDTH, VPHEIGHT);
	glMatrixMode(GL_PROJECTION);

	if (!game.paused) {	// If the game is active, the movement of the pieces is carried out

		glLoadIdentity();
		gluOrtho2D(0, COLS, ROWS, 0);

		for (int r = 0; r < ROWS; r++) {
			for (int c = 0; c < COLS; c++) {
				Square& square = game.mainGrid[r][c];
				if (square.isFilled) {
					glColor3f(square.red, square.green, square.blue);
					glRectd(c + .1, r + .1, c + .9, r + .9);
				}
				else {
					glColor3f(0.2, 0.2, 0.2);
					glRectd(c, r, c + 1, r + 1);
				}
			}
		}
	}
	else {

		glLoadIdentity();
		gluOrtho2D(0, VPWIDTH, 0, VPHEIGHT);

		if (game.paused && !game.killed) {		// If the game is paused, the pause menu is displayed
			glColor3f(1, 1, 1);
			sprintf_s(msg, N, "GAME PAUSED");
			BitmapText(msg, 140, VPHEIGHT / 2);
		}
		if (game.paused && game.killed) {		// If the game is over, the restart menu is displayed
			glColor3f(1, 1, 1);
			sprintf_s(msg, N, "GAME OVER");
			BitmapText(msg, 155, VPHEIGHT / 2 + 50);
			sprintf_s(msg, N, "YOUR SCORE: %d", game.linesCleared);
			BitmapText(msg, 140, VPHEIGHT / 2);
			sprintf_s(msg, N, "Press [ENTER] to restart ...");
			BitmapText(msg, 75, VPHEIGHT / 2 - 100);
		}
	}

	// Vertical dividing strip
	glViewport(VPWIDTH, 0, VPWIDTH, VPHEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, VPWIDTH, 0, VPHEIGHT);

	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2d(1, 0);
	glVertex2d(1, glutGet(GLUT_WINDOW_HEIGHT));
	glEnd();

	// Messages on the right side of the screen
	glLoadIdentity();
	gluOrtho2D(0, VPWIDTH, 0, VPHEIGHT);

	glColor3f(1, 1, 1);
	sprintf_s(msg, N, "Lines Cleared: %d", game.linesCleared);
	BitmapText(msg, 50, 100);
	sprintf_s(msg, N, "Shapes Encountered: %d", game.shapesCount);
	BitmapText(msg, 50, 50);
	sprintf_s(msg, N, "Next Shape:");
	BitmapText(msg, 50, VPHEIGHT - 50);

	// Vertical dividing strip
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2d(1, 0);
	glVertex2d(1, glutGet(GLUT_WINDOW_HEIGHT));
	glEnd();

	// Grid displaying the next piece
	glViewport(VPWIDTH + 50, -50, VPWIDTH, VPHEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, COLS, ROWS, 0);

	for (int r = 1; r < 5; r++) {
		for (int c = 0; c < 2; c++) {
			Square& square = game.nextPieceGrid[r][c];
			if (square.isFilled) {
				glColor3f(square.red, square.green, square.blue);
				glRectd(c + .1, r + .1, c + .9, r + .9);
			}
			else {
				glColor3f(0.2, 0.2, 0.2);
				glRectd(c, r, c + 1, r + 1);
			}
		}
	}

	glutSwapBuffers();
}

void main(int argc, char* argv[])
{
	srand(time(0));
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(VPWIDTH * 2, VPHEIGHT);

	glutCreateWindow("Tetris");

	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(game.timer, timer, 0);

	glutMainLoop();
}