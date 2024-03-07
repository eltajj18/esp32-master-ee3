// Tic-tac-toe playing AI. Exhaustive tree-search. WTFPL
// Matthew Steel 2009, www.www.repsilat.com
// https://gist.github.com/MatthewSteel/3158579
#include <stdio.h>

void draw(int b[9])
{
    printf(" %c | %c | %c\n", (b[0]), (b[1]), (b[2]));
    printf("---+---+---\n");
    printf(" %c | %c | %c\n", (b[3]), (b[4]), (b[5]));
    printf("---+---+---\n");
    printf(" %c | %c | %c\n", (b[6]), (b[7]), (b[8]));
}

int win(const int board[9])
{
    unsigned wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
    for (int i = 0; i < 8; ++i)
    {
        if (board[wins[i][0]] != ' ' && board[wins[i][0]] == board[wins[i][1]] && board[wins[i][0]] == board[wins[i][2]])
            return board[wins[i][2]];
    }
    return 0;
}

int minimax(int board[9], int player)
{
    int winner = win(board);
    if (winner != 0)
        return winner * player;

    int move = -1;
    int score = -2;
    for (int i = 0; i < 9; ++i)
    {
        if (board[i] == ' ')
        {
            board[i] = player;
            int thisScore = -minimax(board, player * -1);
            if (thisScore > score)
            {
                score = thisScore;
                move = i;
            }
            board[i] = ' ';
        }
    }
    if (move == -1)
        return 0;
    return score;
}

void computerMove(int board[9])
{
    int move = -1;
    int score = -2;
    for (int i = 0; i < 9; ++i)
    {
        if (board[i] == ' ')
        {
            board[i] = 1;
            int tempScore = -minimax(board, -1);
            board[i] = 'O';
            if (tempScore > score)
            {
                score = tempScore;
                move = i;
            }
        }
    }
    board[move] = 'O';
}

int main()
{    int board[9] = {'X', ' ', 'O', 'X', ' ', 'O', 'X', ' ', 'O'};
    draw(board);

    if (win(board) != 0)
    {
        printf("The game has already ended.\n");
        return 0;
    }

    computerMove(board);

    printf("Computer's move:\n");
    draw(board);

    if (win(board) != 0)
    {
        printf("Computer wins!\n");
    }
    else
    {
        printf("The game continues...\n");
    }

    return 0;
}
