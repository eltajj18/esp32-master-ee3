#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdbool.h>

extern char PLAYER_MOVE;
extern char COMPUTER_MOVE;

void initialize_game_moves();
int configureMoves(char board[3][3], char *firstMoveSymbol);
bool isComputerMoveO();

#endif // GAME_CONFIG_H