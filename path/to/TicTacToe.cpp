#include <iostream>

using namespace std;

const int SIZE = 3;
char board[SIZE][SIZE] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}};
int turn = 0;

void printBoard() {
    for (int i = 0; i < SIZE; i++) {
        cout << board[i][0] << " | " << board[i][1] << " | " << board[i][2] << "\n";
        if (i < SIZE - 1) cout << "---------+---------+---------\n";
    }
}

bool checkWin(int player) {
    for (int i = 0; i < SIZE; i++) {
        if (board[i][0] == board[i][1] && board[i][2] == board[i][1]) return true;
        if (board[0][i] == board[1][i] && board[2][i] == board[1][i]) return true;
    }
    if ((board[0][0] == board[1][1] && board[2][2] == board[1][1]) || (board[0][2] == board[1][1] && board[2][0] == board[1][1])) return true;
    return false;
}

void computerMove() {
    while (true) {
        int row = rand() % 3, col = rand() % 3;
        if (board[row][col] != 'X' && board[row][col] != 'O') {
            board[row][col] = 'O';
            break;
        }
    }
}

int main() {
    srand(time(0));
    while (true) {
        printBoard();
        if (turn % 2 == 1) {
            // player move
            int row, col;
            cout << "Enter position: "; cin >> row >> col;
            board[row - 1][col - 1] = 'X';
            turn++;
        } else {
            computerMove();
            turn++;
        }
        if (checkWin('X')) {
            printBoard();
            cout << "Player wins!\n";
            break;
        } else if (checkWin('O')) {
            printBoard();
            cout << "Computer wins!\n";
            break;
        }
    }
    return 0;
}