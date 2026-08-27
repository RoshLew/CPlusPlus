#include <iostream>
#include <iomanip>
#include <cctype>

int main() {
    std::cout << "ASCII Table (0–127)\n\n";
    std::cout << std::left << std::setw(10) << "DEC"
              << std::setw(10) << "HEX"
              << "CHAR\n";
    std::cout << "---------------------------\n";

    for (int i = 0; i < 128; ++i) {
        std::cout << std::left << std::setw(10) << i
                  << std::setw(10) << std::hex << std::uppercase << i << std::dec;

        if (std::isprint(i))
            std::cout << static_cast<char>(i);
        else
            std::cout << "(ctrl)";

        std::cout << "\n";
    }

    return 0;
}