#include <iostream>
#include <limits>
#include <iomanip>

template <typename T>
void print_limits(const std::string& name) {
    std::cout << std::left << std::setw(20) << name
              << " min = " << std::numeric_limits<T>::lowest()
              << ", max = " << std::numeric_limits<T>::max()
              << "\n";
}

int main() {
    std::cout << std::boolalpha;

    print_limits<bool>("bool");

    print_limits<char>("char");
    print_limits<signed char>("signed char");
    print_limits<unsigned char>("unsigned char");

    print_limits<short>("short");
    print_limits<unsigned short>("unsigned short");

    print_limits<int>("int");
    print_limits<unsigned int>("unsigned int");

    print_limits<long>("long");
    print_limits<unsigned long>("unsigned long");

    print_limits<long long>("long long");
    print_limits<unsigned long long>("unsigned long long");

    print_limits<float>("float");
    print_limits<double>("double");
    print_limits<long double>("long double");

    return 0;
}
