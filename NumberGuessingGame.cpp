// NumberGuessingGame.cpp
// A tiny console game for a brand-new C++ learner.
//
// What you'll practice in this one file:
//   - #include / using
//   - variables and constants
//   - cin / cout
//   - a do/while loop
//   - if / else if / else
//   - a tiny helper function
//
// Compile:  g++ NumberGuessingGame.cpp -o NumberGuessingGame.exe -std=c++17
// Run:      NumberGuessingGame.exe

#include <iostream>     // for std::cin / std::cout
#include <cstdlib>      // for std::rand / std::srand
#include <ctime>        // for std::time

// --- Settings you can tweak ---
const int  MIN_NUMBER = 1;
const int  MAX_NUMBER = 100;
const int  MAX_TRIES  = 7;     // 0 = unlimited

// Returns true if the player wants to play again.
bool askToPlayAgain() {
    std::cout << "\nPlay again? (y/n): ";
    char answer;
    std::cin >> answer;
    return answer == 'y' || answer == 'Y';
}

int main() {
    // Seed the random number generator with the current time so each run is different.
    std::srand(static_cast<unsigned int>(std::time(0)));

    std::cout << "==============================\n";
    std::cout << "  Number Guessing Game\n";
    std::cout << "  Guess a number between "
              << MIN_NUMBER << " and " << MAX_NUMBER << ".\n";
    std::cout << "  You have " << MAX_TRIES << " tries.\n";
    std::cout << "==============================\n";

    // Outer loop: one full game per iteration.
    bool playing = true;
    while (playing) {
        // Pick a secret number in [MIN_NUMBER, MAX_NUMBER].
        int secret = MIN_NUMBER + std::rand() % (MAX_NUMBER - MIN_NUMBER + 1);

        int tries = 0;
        int guess = 0;
        bool won  = false;

        std::cout << "\nI'm thinking of a number. What is it?\n";

        // Inner loop: keep asking until they get it or run out of tries.
        while (!won && (MAX_TRIES == 0 || tries < MAX_TRIES)) {
            std::cout << "Your guess: ";
            std::cin >> guess;
            ++tries;

            if      (guess < secret) std::cout << "  Too low!\n";
            else if (guess > secret) std::cout << "  Too high!\n";
            else {
                std::cout << "  Correct! You got it in " << tries << " tries.\n";
                won = true;
            }
        }

        // If they never won, reveal the answer.
        if (!won) {
            std::cout << "  Out of tries! The number was " << secret << ".\n";
        }

        playing = askToPlayAgain();
    }

    std::cout << "Thanks for playing!\n";
    return 0;
}
