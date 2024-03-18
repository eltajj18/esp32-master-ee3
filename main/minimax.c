#include <stdio.h>
#include <stdbool.h>

#define PLAYER 'X'
#define COMPUTER 'O'

// Function to evaluate the score of the board
int evaluate(char board[3][3])
{
    // Checking for Rows for X or O victory.
    for (int row = 0; row < 3; row++)
    {
        if (board[row][0] == board[row][1] &&
            board[row][1] == board[row][2])
        {
            if (board[row][0] == COMPUTER)
                return +10;
            else if (board[row][0] == PLAYER)
                return -10;
        }
    }

    // Checking for Columns for X or O victory.
    for (int col = 0; col < 3; col++)
    {
        if (board[0][col] == board[1][col] &&
            board[1][col] == board[2][col])
        {
            if (board[0][col] == COMPUTER)
                return +10;
            else if (board[0][col] == PLAYER)
                return -10;
        }
    }

    // Checking for Diagonals for X or O victory.
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        if (board[0][0] == COMPUTER)
            return +10;
        else if (board[0][0] == PLAYER)
            return -10;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0])
    {
        if (board[0][2] == COMPUTER)
            return +10;
        else if (board[0][2] == PLAYER)
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
                    board[i][j] = COMPUTER;

                    // Call minimax recursively and choose the maximum value
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
                    board[i][j] = PLAYER;

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

// Struct to represent a move
typedef struct Move
{
    int row, col;
} Move;

// Function to find the best move for the computer
Move findBestMove(char board[3][3])
{
    int bestVal = -1000;

    Move bestMove = {-1, -1}; // Initialize the best move with invalid position

    // Traverse all cells, evaluate minimax function for all empty cells, and return the cell with optimal value.
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            // Check if cell is empty
            if (board[i][j] == ' ')
            {
                // Make the move
                board[i][j] = COMPUTER;

                // Compute evaluation function for this move.
                int moveVal = minimax(board, 0, false);

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

// int main()
// {
//     char board[3][3] = {
//         {'X', 'X', 'O'},
//         {'O', 'O', 'X'},
//         {'X', 'X', 'O'}};
//     int x = evaluate(board);
//     printf("%d\n", x);
//     int y = minimax(board, 0, false);
//     printf("%d\n", y);
//     Move bestMove = findBestMove(board);
//     printf("The best move for the computer is:\n");
//     printf("Row: %d, Column: %d\n", bestMove.row, bestMove.col);

//     return 0;
// }
