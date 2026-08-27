// TicTacToe - Console version with simple AI
// Compile: g++ TicTacToe1.cpp -o TicTacToe1.exe -O2 -std=c++17

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

using namespace std;

const int BOARD_SIZE = 3;
char board[BOARD_SIZE][BOARD_SIZE];
bool gameActive = true;
mt19937 rng;

// Initialize board with empty spaces
void initBoard() {
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board[i][j] = ' ';
}

// Draw the board to console
void drawBoard() {
    cout << "\n";
    for (int i = 0; i < BOARD_SIZE; ++i) {
        cout << "  ";
        for (int j = 0; j < BOARD_SIZE; ++j) {
            cout << " " << (board[i][j] == ' ' ? char('1' + i * 3 + j) : board[i][j]) << " ";
            if (j < BOARD_SIZE - 1) cout << "|";
        }
        cout << "\n";
        if (i < BOARD_SIZE - 1) cout << "  ---+---+---\n";
    }
    cout << "\n";
}

// Check if player has won
bool checkWin(char player) {
    // Rows and columns
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
    }
    // Diagonals
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;
    return false;
}

// Check if board is full (draw)
bool allFilled() {
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j] == ' ') return false;
    return true;
}

// Get player move (1-9)
bool getPlayerMove(int& row, int& col) {
    int move;
    cout << "Enter your move (1-9): ";
    if (!(cin >> move)) {
        cin.clear();
        cin.ignore(10000, '\n');
        return false;
    }
    if (move < 1 || move > 9) return false;
    row = (move - 1) / 3;
    col = (move - 1) % 3;
    if (board[row][col] != ' ') return false;
    return true;
}

// Simple AI: win > block > center > corner > random
void computerMove() {
    // Try to win
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == ' ') {
                board[i][j] = 'X';
                if (checkWin('X')) return;
                board[i][j] = ' ';
            }
        }
    }
    // Block player
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == ' ') {
                board[i][j] = 'O';
                if (checkWin('O')) {
                    board[i][j] = 'X';
                    return;
                }
                board[i][j] = ' ';
            }
        }
    }
    // Center
    if (board[1][1] == ' ') { board[1][1] = 'X'; return; }
    // Corners
    vector<pair<int,int>> corners = {{0,0}, {0,2}, {2,0}, {2,2}};
    shuffle(corners.begin(), corners.end(), rng);
    for (auto [r, c] : corners) {
        if (board[r][c] == ' ') { board[r][c] = 'X'; return; }
    }
    // Any empty
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j] == ' ') { board[i][j] = 'X'; return; }
}

int main() {
    rng.seed(static_cast<unsigned int>(time(0)));
    initBoard();

    cout << "========== TIC TAC TOE ==========\n";
    cout << "You are 'O', Computer is 'X'\n";
    cout << "Enter moves as numbers 1-9:\n";
    cout << " 1 | 2 | 3 \n";
    cout << "---+---+---\n";
    cout << " 4 | 5 | 6 \n";
    cout << "---+---+---\n";
    cout << " 7 | 8 | 9 \n\n";

    while (gameActive) {
        drawBoard();

        // Player's turn
        int row, col;
        while (!getPlayerMove(row, col)) {
            cout << "Invalid move. Try again (1-9): ";
        }
        board[row][col] = 'O';

        if (checkWin('O')) {
            drawBoard();
            cout << "You win! 🎉\n";
            break;
        }
        if (allFilled()) {
            drawBoard();
            cout << "It's a draw!\n";
            break;
        }

        // Computer's turn
        cout << "Computer is thinking...\n";
        computerMove();

        if (checkWin('X')) {
            drawBoard();
            cout << "Computer wins! 🤖\n";
            break;
        }
        if (allFilled()) {
            drawBoard();
            cout << "It's a draw!\n";
            break;
        }
    }

    return 0;
}