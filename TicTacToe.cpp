//compile: g++ TicTacToe.cpp -o TicTacToe.exe -O2 -std=c++17
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

// Define the size of the board
const int BOARD_SIZE = 3;

// Define the possible states for each cell in the board
enum CellState { EMPTY, X, O };

// Define a structure to represent a single cell on the board
struct Cell {
    int row;
    int col;
    CellState state;
};

// Draw the game board
void DrawGameBoard(const vector<vector<Cell>>& board) {
    cout << "\n";
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            cout << " ";
            if (board[i][j].state == X) cout << "X";
            else if (board[i][j].state == O) cout << "O";
            else cout << " ";
            if (j < BOARD_SIZE - 1) cout << " |";
        }
        cout << "\n";
        if (i < BOARD_SIZE - 1) cout << "---+---+---\n";
    }
    cout << "\n";
}

// Prompt the current player to make a move and apply it
void MakeMove(vector<vector<Cell>>& board, CellState currentPlayer) {
    int row, col;
    while (true) {
        cout << "Player " << (currentPlayer == X ? 'X' : 'O') << ", enter row and column (1-" << BOARD_SIZE << "): ";
        if (!(cin >> row >> col)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter two numbers." << endl;
            continue;
        }
        if (row >= 1 && row <= BOARD_SIZE && col >= 1 && col <= BOARD_SIZE) {
            if (board[row-1][col-1].state == EMPTY) {
                board[row-1][col-1].state = currentPlayer;
                board[row-1][col-1].row = row-1;
                board[row-1][col-1].col = col-1;
                break;
            } else {
                cout << "Cell already occupied. Try again." << endl;
            }
        } else {
            cout << "Coordinates out of range. Use 1-" << BOARD_SIZE << "." << endl;
        }
    }
}

// Check for a win; return X or O if there's a winner, otherwise EMPTY
CellState CheckWinner(const vector<vector<Cell>>& board) {
    // rows and columns
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board[i][0].state != EMPTY &&
            board[i][0].state == board[i][1].state &&
            board[i][1].state == board[i][2].state) return board[i][0].state;
        if (board[0][i].state != EMPTY &&
            board[0][i].state == board[1][i].state &&
            board[1][i].state == board[2][i].state) return board[0][i].state;
    }
    // diagonals
    if (board[0][0].state != EMPTY && board[0][0].state == board[1][1].state && board[1][1].state == board[2][2].state)
        return board[0][0].state;
    if (board[0][2].state != EMPTY && board[0][2].state == board[1][1].state && board[1][1].state == board[2][0].state)
        return board[0][2].state;

    return EMPTY;
}

bool IsBoardFull(const vector<vector<Cell>>& board) {
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j].state == EMPTY) return false;
    return true;
}

int main() {
    // Initialize the game board with empty cells
    vector<vector<Cell>> board(BOARD_SIZE, vector<Cell>(BOARD_SIZE));
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board[i][j] = Cell{ i, j, EMPTY };

    // Set the current player to X
    CellState currentPlayer = X;

    // Game loop
    while (true) {
        DrawGameBoard(board);
        MakeMove(board, currentPlayer);
        CellState winner = CheckWinner(board);
        if (winner != EMPTY) {
            DrawGameBoard(board);
            cout << "Player " << (winner == X ? 'X' : 'O') << " wins!\n";
            break;
        }
        if (IsBoardFull(board)) {
            DrawGameBoard(board);
            cout << "It's a draw!\n";
            break;
        }
        // switch player
        currentPlayer = (currentPlayer == X) ? O : X;
    }

    return 0;
}