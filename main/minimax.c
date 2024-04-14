#include <stdio.h>
#include <stdbool.h>
#include "include/minimax.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h> // For srand()
#include <time.h>   // For time()
#include "include/game_config.h"

// Function to evaluate the score of the board
void draw(char b[3][3])
{
    printf(" %c | %c | %c\n", b[0][0], b[0][1], b[0][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c\n", b[1][0], b[1][1], b[1][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c\n", b[2][0], b[2][1], b[2][2]);
}
bool isBoardEmpty(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] != ' ') {
                return false;
            }
        }
    }
    return true;
}

int evaluate(char board[3][3])
{
    // Checking for Rows for X or O victory.
    for (int row = 0; row < 3; row++)
    {
        if (board[row][0] == board[row][1] &&
            board[row][1] == board[row][2])
        {
            if (board[row][0] == COMPUTER_MOVE)
                return +10;
            else if (board[row][0] == PLAYER_MOVE)
                return -10;
        }
    }

    // Checking for Columns for X or O victory.
    for (int col = 0; col < 3; col++)
    {
        if (board[0][col] == board[1][col] &&
            board[1][col] == board[2][col])
        {
            if (board[0][col] == COMPUTER_MOVE)
                return +10;
            else if (board[0][col] == PLAYER_MOVE)
                return -10;
        }
    }

    // Checking for Diagonals for X or O victory.
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        if (board[0][0] == COMPUTER_MOVE)
            return +10;
        else if (board[0][0] == PLAYER_MOVE)
            return -10;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0])
    {
        if (board[0][2] == COMPUTER_MOVE)
            return +10;
        else if (board[0][2] == PLAYER_MOVE)
            return -10;
    }

    // Else if none of them have won then return 0
    return 0;
}

// Function to check if there are moves left on the board
bool isMovesLeft(char board[3][3])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ')
                return true;
    return false;
}

// The minimax function considers all the possible ways the game can go and returns the value of the board
int minimax(char board[3][3], int depth, bool isMax)
{
    // vTaskDelay(200 / portTICK_PERIOD_MS);
    int score = evaluate(board);
    // If Maximizer has won the game return his/her evaluated score
    if (score == 10)
        return score;

    // If Minimizer has won the game return his/her evaluated score
    if (score == -10)
        return score;

    // If there are no more moves and no winner then it is a tie
    if (!isMovesLeft(board))
        return 0;

    // If this maximizer's move
    if (isMax)
    {
        int best = -1000;

        // Traverse all cells
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                // Check if cell is empty
                if (board[i][j] == ' ')
                {
                    // Make the move
                    board[i][j] = COMPUTER_MOVE;

                    // Call minimax recursively and choose the maximum value
                    // vTaskDelay(20 / portTICK_PERIOD_MS);
                    best = (best > minimax(board, depth + 1, !isMax)) ? best : minimax(board, depth + 1, !isMax);

                    // Undo the move
                    board[i][j] = ' ';
                }
            }
        }
        return best;
    }

    // If this minimizer's move
    else
    {
        int best = 1000;

        // Traverse all cells
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                // Check if cell is empty
                if (board[i][j] == ' ')
                {
                    // Make the move
                    board[i][j] = PLAYER_MOVE;

                    // Call minimax recursively and choose the minimum value
                    best = (best < minimax(board, depth + 1, !isMax)) ? best : minimax(board, depth + 1, !isMax);

                    // Undo the move
                    board[i][j] = ' ';
                }
            }
        }
        return best;
    }
}

// Function to find the best move for the computer
Move findBestMove(char board[3][3])
{
    if (isBoardEmpty(board)) {
    Move randomMove;
    // Board is entirely empty, so any random cell is guaranteed to be empty
    randomMove.row = rand() % 3; // Random row, 0 to 2
    randomMove.col = rand() % 3; // Random column, 0 to 2
    return randomMove;
}
else{
    
    int bestVal = -1000;
    Move bestMove = {-1, -1}; // Initialize the best move with invalid position
    vTaskDelay(200 / portTICK_PERIOD_MS);
    // Traverse all cells, evaluate minimax function for all empty cells, and return the cell with optimal value.
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            // Check if cell is empty
            if (board[i][j] == ' ')
            {
                // Make the move
                board[i][j] = COMPUTER_MOVE;

                // Compute evaluation function for this move.
                // int moveVal = minimax(board, 0, false);
                // vTaskDelay(20 / portTICK_PERIOD_MS);
                int moveVal = minimax(board, 1, false);

                // Undo the move
                board[i][j] = ' ';

                // If the value of the current move is more than the best value, update the best move
                if (moveVal > bestVal)
                {
                    bestMove.row = i;
                    bestMove.col = j;
                    bestVal = moveVal;
                }
            }
        }
    }

    return bestMove;
    }
}


void transformArrayTo3x3(char *gamestate, char board[3][3])
{
    for (int i = 0; i < 9; ++i)
    {
        // Convert the string to a char and assign to the correct position in the board
        // Note: Since each string is known to be a single character, we directly access it with [0]
        board[i / 3][i % 3] = gamestate[i]; // Dividing and modulo by 3 maps the index to 2D coordinates
    }
}

// Function to find a random empty move
Move findRandomEmptyMove(char board[3][3])
{
    Move move;
    int emptyPositions = 0;
    int chosenPosition;
    int count = 0;

    // First, count the number of empty positions
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board[i][j] == ' ')
            {
                emptyPositions++;
            }
        }
    }

    if (emptyPositions > 0)
    {
        // Choose a random empty position
        chosenPosition = rand() % emptyPositions;

        // Find the chosen empty position
        for (int i = 0; i < 3 && count <= chosenPosition; i++)
        {
            for (int j = 0; j < 3 && count <= chosenPosition; j++)
            {
                if (board[i][j] == ' ')
                {
                    if (count == chosenPosition)
                    {
                        move.row = i;
                        move.col = j;
                        return move;
                    }
                    count++;
                }
            }
        }
    }

    // In case the board is full, return an invalid move
    move.row = -1;
    move.col = -1;
    return move;
}

// Medium difficulty function
Move medium(char board[3][3])
{
    // Seed the random number generator
    // Note: Normally, you'd call srand(time(NULL)) once at the start of your program
    srand(time(NULL)); // Call this once in the main function or initialization code instead

    Move move;
    if (rand() % 2)
    {
        printf("Computer chose the best move\n"); // Generates 0 or 1 randomly
        move = findBestMove(board);               // Use the best move
    }
    else
    {   printf("Computer chose a random move\n");
        move = findRandomEmptyMove(board); // Make a random move
    }
    return move;
}