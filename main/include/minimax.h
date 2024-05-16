#ifndef MINIMAX_H
#define MINIMAX_H

// Define the struct Move here since it's used as a return type for findBestMove
typedef struct Move {
    int row, col;
} Move;

// Declaration of functions defined in minimax.c that are available to other files
int evaluate(char board[3][3]);
bool isMovesLeft(char board[3][3]);
int minimax(char board[3][3], int depth, bool isMax);
Move findBestMove(char board[3][3]);
void transformArrayTo3x3(char* gamestate, char board[3][3]);
void draw(char b[3][3]);
bool isBoardEmpty(char board[3][3]);
Move findRandomEmptyMove(char board[3][3]);
Move medium(char board[3][3]);

#endif // MINIMAX_H
