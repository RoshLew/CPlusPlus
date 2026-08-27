// Graphical Tic-Tac-Toe Game - Player vs Computer
// Compile: g++ TicTacToeGraphical.cpp -o TicTacToeGraphical -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system
// Requires: SFML library (https://www.sfml-dev.org/)

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

// Game Constants
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 700;
const int CELL_SIZE = 180;
const int GRID_OFFSET_X = 30;
const int GRID_OFFSET_Y = 50;
const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 50;

// Game States
enum GameState { PLAYING, PLAYER_WIN, COMPUTER_WIN, DRAW };

class TicTacToe {
private:
    char board[3][3];
    GameState gameState;
    bool playerTurn;
    int cellSize;
    sf::Font font;
    sf::RenderWindow window;

public:
    TicTacToe() : gameState(PLAYING), playerTurn(true), cellSize(CELL_SIZE),
                  window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Tic-Tac-Toe") {
        srand(static_cast<unsigned>(time(0)));
        initializeBoard();
        window.setFramerateLimit(60);
        
        // Try to load a system font
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            cerr << "Warning: Could not load font. Using default rendering." << endl;
        }
    }

    void initializeBoard() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = ' ';
            }
        }
        gameState = PLAYING;
        playerTurn = true;
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                int x = event.mouseButton.x;
                int y = event.mouseButton.y;
                if (gameState == PLAYING && playerTurn) {
                    handlePlayerMove(x, y);
                } else if (gameState != PLAYING) {
                    handleResetButtonClick(x, y);
                }
            }
        }
    }

    void handleResetButtonClick(int x, int y) {
        if (x > WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2 &&
            x < WINDOW_WIDTH / 2 + BUTTON_WIDTH / 2 &&
            y > WINDOW_HEIGHT - 50 &&
            y < WINDOW_HEIGHT - 50 + BUTTON_HEIGHT) {
            initializeBoard();
        }
    }

    void handlePlayerMove(int x, int y) {
        // Check if click is within the game board
        if (x < GRID_OFFSET_X || x > GRID_OFFSET_X + 3 * CELL_SIZE ||
            y < GRID_OFFSET_Y || y > GRID_OFFSET_Y + 3 * CELL_SIZE) {
            return;
        }

        int col = (x - GRID_OFFSET_X) / CELL_SIZE;
        int row = (y - GRID_OFFSET_Y) / CELL_SIZE;

        if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
            board[row][col] = 'O';  // Player is O
            playerTurn = false;

            if (checkWin('O')) {
                gameState = PLAYER_WIN;
            } else if (isBoardFull()) {
                gameState = DRAW;
            }
        }
    }

    void update() {
        if (!playerTurn && gameState == PLAYING) {
            makeComputerMove();

            if (checkWin('X')) {
                gameState = COMPUTER_WIN;
            } else if (isBoardFull()) {
                gameState = DRAW;
            }

            playerTurn = true;
        }
    }

    void makeComputerMove() {
        // Strategy: Try to win, block player, or take center/corners
        int move = findBestMove();
        if (move != -1) {
            int row = move / 3;
            int col = move % 3;
            board[row][col] = 'X';  // Computer is X
        }
    }

    int findBestMove() {
        // Check if computer can win
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (board[row][col] == ' ') {
                board[row][col] = 'X';
                if (checkWin('X')) {
                    board[row][col] = ' ';
                    return i;
                }
                board[row][col] = ' ';
            }
        }

        // Check if need to block player
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (board[row][col] == ' ') {
                board[row][col] = 'O';
                if (checkWin('O')) {
                    board[row][col] = ' ';
                    return i;
                }
                board[row][col] = ' ';
            }
        }

        // Take center if available
        if (board[1][1] == ' ') return 4;

        // Take corners
        int corners[] = {0, 2, 6, 8};
        for (int corner : corners) {
            int row = corner / 3, col = corner % 3;
            if (board[row][col] == ' ') return corner;
        }

        // Take any available space
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (board[row][col] == ' ') return i;
        }

        return -1;
    }

    bool checkWin(char player) {
        // Check rows
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
                return true;
        }

        // Check columns
        for (int j = 0; j < 3; j++) {
            if (board[0][j] == player && board[1][j] == player && board[2][j] == player)
                return true;
        }

        // Check diagonals
        if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
            return true;
        if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
            return true;

        return false;
    }

    bool isBoardFull() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') return false;
            }
        }
        return true;
    }

    void render() {
        window.clear(sf::Color::White);

        // Draw grid
        drawGrid();

        // Draw pieces
        drawPieces();

        // Draw game status and buttons
        drawUI();

        window.display();
    }

    void drawGrid() {
        sf::Color gridColor(100, 100, 100);
        
        // Vertical lines
        for (int i = 1; i < 3; i++) {
            sf::RectangleShape line(sf::Vector2f(2, 3 * CELL_SIZE));
            line.setPosition(GRID_OFFSET_X + i * CELL_SIZE, GRID_OFFSET_Y);
            line.setFillColor(gridColor);
            window.draw(line);
        }

        // Horizontal lines
        for (int i = 1; i < 3; i++) {
            sf::RectangleShape line(sf::Vector2f(3 * CELL_SIZE, 2));
            line.setPosition(GRID_OFFSET_X, GRID_OFFSET_Y + i * CELL_SIZE);
            line.setFillColor(gridColor);
            window.draw(line);
        }
    }

    void drawPieces() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int x = GRID_OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2;
                int y = GRID_OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2;

                if (board[i][j] == 'X') {
                    drawX(x, y);
                } else if (board[i][j] == 'O') {
                    drawO(x, y);
                }
            }
        }
    }

    void drawX(int cx, int cy) {
        sf::Color xColor(220, 50, 50);  // Red

        // First diagonal (top-left to bottom-right)
        sf::RectangleShape line1(sf::Vector2f(5, 100));
        line1.setOrigin(2.5f, 50);  // Center the origin
        line1.setPosition(cx, cy);
        line1.setFillColor(xColor);
        line1.setRotation(45);
        window.draw(line1);

        // Second diagonal (top-right to bottom-left)
        sf::RectangleShape line2(sf::Vector2f(5, 100));
        line2.setOrigin(2.5f, 50);  // Center the origin
        line2.setPosition(cx, cy);
        line2.setFillColor(xColor);
        line2.setRotation(-45);
        window.draw(line2);
    }

    void drawO(int cx, int cy) {
        int radius = 50;
        sf::Color oColor(50, 150, 220);  // Blue

        sf::CircleShape circle(radius);
        circle.setPosition(cx - radius, cy - radius);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(5);
        circle.setOutlineColor(oColor);
        window.draw(circle);
    }

    void drawUI() {
        string message;
        sf::Color textColor = sf::Color::Black;

        if (gameState == PLAYER_WIN) {
            message = "You Win! Congratulations!";
            textColor = sf::Color::Green;
        } else if (gameState == COMPUTER_WIN) {
            message = "Computer Wins! Better luck next time.";
            textColor = sf::Color::Red;
        } else if (gameState == DRAW) {
            message = "It's a Draw!";
            textColor = sf::Color(200, 140, 0);  // Amber - visible on white
        } else {
            message = playerTurn ? "Your Turn (O)" : "Computer's Turn (X)";
        }

        sf::Text text(message, font, 20);
        text.setFillColor(textColor);
        text.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 100);
        window.draw(text);

        // Draw reset button if game is over
        if (gameState != PLAYING) {
            drawButton(WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2, WINDOW_HEIGHT - 50,
                      BUTTON_WIDTH, BUTTON_HEIGHT, "New Game");
        }
    }

    void drawButton(int x, int y, int width, int height, const string& text) {
        sf::RectangleShape button(sf::Vector2f(width, height));
        button.setPosition(x, y);
        button.setFillColor(sf::Color::Cyan);
        button.setOutlineThickness(2);
        button.setOutlineColor(sf::Color::Black);
        window.draw(button);

        sf::Text buttonText(text, font, 18);
        buttonText.setFillColor(sf::Color::Black);
        buttonText.setPosition(x + 20, y + 10);
        window.draw(buttonText);
    }
};

int main() {
    TicTacToe game;
    game.run();
    return 0;
}
