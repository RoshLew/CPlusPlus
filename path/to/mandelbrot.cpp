#include <iostream>
#include <cmath>
#include <string>

int main() {
    const int width = 800;
    const int height = 600;
    const int maxIter = 256;

    double minX = -2.5; 
    double maxX = 1.5;
    double minY = -1.5;
    double maxY = 1.5;

    // Function to calculate the Mandelbrot set for a given point
    auto mandelbrot = [&](double cX, double cY) {
        double zX = 0.0;
        double zY = 0.0;
            int iter = 0;
        while (iter < maxIter && (zX * zX + zY * zY) < 4.0) {
            double temp = zX * zX - zY * zY + cX;
            zY = 2.0 * zX * zY + cY;
            zX = temp;
            ++iter;
        }
        return iter;
    };

    // Function to convert iteration count to RGB values
    auto colorize = [&](int iter) {
        if (iter == maxIter)
            return "\033[30m"; // Black for points outside the set

        float t = static_cast<float>(iter) / maxIter;
        int r, g, b;

        // HSV to RGB conversion
        float h = 240.0 * t; // Shift hue by 120 degrees
        float f = h - floor(h);
        if (f < 1/6.0) {
            r = static_cast<int>(3*255*f); g = 0; b = static_cast<int>(3*255*(1-f));
} else if (f < 2/6.0) {
    r = 255; g = static_cast<int>(3*255*f); b = 0;
} else if (f < 3/6.0) {
    r = 255 - static_cast<int>(3*255*(f-1/6.0)); g = 255; b = 0;
} else if (f < 4/6.0) {
    r = 0; g = 255; b = static_cast<int>(3*255*f);
} else if (f < 5/6.0) {
    r = 0; g = 255 - static_cast<int>(3*255*(f-4/6.0));
    b = static_cast<int>(3*255*(1-f));
} else {
    r = static_cast<int>(3*255*(1-f)); g = 0;
        }
return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
    };
      // Rest of the code remains unchanged
    };

    // Main loop to generate and display the Mandelbrot fractal
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            double cX = minX + (double)x / width * (maxX - minX);
            double cY = minY + (double)y / height * (maxY - minY);

            int color = mandelbrot(cX, cY);
            std::cout << colorize(color) << "*";
        }
        std::cout << "\n";
    }

    return 0;
}

