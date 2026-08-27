#include <windows.h>
#include <iostream>

const int BOARD_SIZE = 3;

struct Player {
    char mark;
};

class TicTacToe {
public:
    TicTacToe() : currentPlayer('X') {}

    void initializeBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = ' ';
        }
    }
}

    void printBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
    system("cls");
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
            std::cout << board[i][j];
        }
        std::cout << std::endl;
    }
}

    bool placeMark(char board[BOARD_SIZE][BOARD_SIZE], int row, int col, char player) {
        if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && board[row][col] == ' ') {
        board[row][col] = player;
        return true;
    }
    return false;
}

    bool checkWin(char board[BOARD_SIZE][BOARD_SIZE], char player) {
        // Check rows and columns for a win
        for (int i = 0; i < BOARD_SIZE; ++i) {
        if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
            (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
            return true;
        }
    }

        // Check diagonals for a win
    if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
        (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
        return true;
    }

    return false;
}

    void playGame() {
        char board[BOARD_SIZE][BOARD_SIZE];
        initializeBoard(board);
    while (true) {
            printBoard(board);
            int row, col;
        std::cout << "Enter your move (row and column numbers separated by space): ";
        std::cin >> row >> col;

            // Handle invalid input
            if (placeMark(board, row, col, currentPlayer)) {
                if (checkWin(board, currentPlayer)) {
                    printBoard(board);
                std::cout << "\nGame Over! Player " << currentPlayer << " wins!\n";
                break;
            }
            currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
        } else {
                std::cout << "Invalid move, please try again.\n";
        }
    }
}

private:
    char currentPlayer;
};

int main() {
    TicTacToe game;
    game.playGame();
    return 0;
}

