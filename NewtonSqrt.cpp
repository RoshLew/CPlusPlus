// Compute square root using Newton/Heron's Method

#include <iostream>
#include <cmath>
using namespace std;


double heron_sqrt(double N, double tolerance = 1e-12, int max_iter = 1000) {
    if (N < 0) {
        cerr << "Error: Negative input not allowed.\n";
        return NAN;
    }

    if (N == 0) return 0;


    double x = N;  // initial guess
    for (int i = 0; i < max_iter; i++) {
        double next = 0.5 * (x + N / x);

        if (fabs(next - x) < tolerance)
            return next;

        x = next;
    }

    return x; // return best approximation if max_iter reached
}

int main() {
    double N;
    cout << "Enter a number to compute its square root: ";
    cin >> N;

    double result = heron_sqrt(N);

    cout.precision(15);
    cout << "Newton/Heron sqrt(" << N << ") = " << result << endl;
    cout << "Check using std::sqrt:      " << sqrt(N) << endl;

    return 0;
}