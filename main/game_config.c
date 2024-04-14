#include "include/game_config.h"
#include <stdio.h>
#include <stdbool.h>

#define EMPTY ' '
char PLAYER_MOVE = 'O';
char COMPUTER_MOVE = 'X';
void initialize_game_moves()
{

    PLAYER_MOVE = 'O';
    COMPUTER_MOVE = 'X';
}
// Function to check the state of the board
int configureMoves(char board[3][3], char *firstMoveSymbol)
{
    int moveCount = 0;
    *firstMoveSymbol = EMPTY; // Initialize with EMPTY

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board[i][j] != EMPTY)
            {
                moveCount++;
                if (moveCount == 1)
                {
                    *firstMoveSymbol = board[i][j];
                }
            }
        }
    }

    if (moveCount == 0)
    {
        COMPUTER_MOVE = 'X';
    }
    else if (moveCount == 1)
    {
        COMPUTER_MOVE = (*firstMoveSymbol == 'X') ? 'O' : 'X';
    }

    return moveCount;
}
bool isComputerMoveO(){
    if (COMPUTER_MOVE == 'O')
    {
        return true;
    }
    else{
        return false;
    }
}
