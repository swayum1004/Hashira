#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

// A simple structure to hold our decoded points.
// Y is now a long double to handle very large numbers.
struct Point {
    long long x;
    long double y;
};

/**
 * @brief Decodes a string value from a given base into a long double.
 * This custom function is needed because std::stoll overflows with large numbers.
 * A long double has a much larger range and can handle the values in the test cases.
 * @param value_str The number as a string (e.g., "aed7015a346d63").
 * @param base The base of the number (e.g., 15).
 * @return The decoded base-10 value as a long double.
 */
long double decode_value(const string& value_str, int base) {
    long double result = 0.0;
    for (char c : value_str) {
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            // Should not happen with valid input
            continue; 
        }

        // Basic error check for invalid digits in the given base
        if (digit >= base) {
            cerr << "Warning: Invalid digit '" << c << "' for base " << base << endl;
            continue;
        }

        result = result * base + digit;
    }
    return result;
}

// Calculates the secret using Lagrange Interpolation at x=0.
// This function remains the same, but now operates on the long double y-values.
long double find_secret(const vector<Point>& points) {
    long double secret = 0.0;
    int k = points.size();

    for (int j = 0; j < k; ++j) {
        long double y_j = points[j].y;
        long double lagrange_basis = 1.0;

        for (int i = 0; i < k; ++i) {
            if (i == j) {
                continue;
            }

            long double numerator = -points[i].x;
            long double denominator = points[j].x - points[i].x;
            
            if (denominator == 0) {
                throw runtime_error("Denominator is zero in Lagrange calculation. X values must be unique.");
            }

            lagrange_basis *= (numerator / denominator);
        }

        secret += (y_j * lagrange_basis);
    }
    return secret;
}

int main() {
    ifstream input_file("input2.json");
    if (!input_file.is_open()) {
        cerr << "Error: Could not open input.json" << endl;
        return 1;
    }

    json data;
    try {
        input_file >> data;
    } catch (json::parse_error& e) {
        cerr << "Error parsing JSON: " << e.what() << endl;
        return 1;
    }

    int k = data["keys"]["k"];
    cout << "Minimum number of points required (k): " << k << endl;

    vector<Point> points;
    int points_count = 0;

    for (auto& element : data.items()) {
        const string& key = element.key();
        const json& val = element.value();

        if (key == "keys") {
            continue;
        }

        if (points_count < k) {
            long long x = stoll(key);
            int base = stoi(val["base"].get<string>());
            string value_str = val["value"].get<string>();
            // Use our new function that returns a long double
            long double y = decode_value(value_str, base);
            
            points.push_back({x, y});
            // Print y values using fixed notation to avoid scientific notation for clarity
            cout << "Parsed point " << points_count + 1 << ": (x=" << x << ", y=" << fixed << y << ")" << endl;
            points_count++;
        } else {
            break;
        }
    }

    if (points.size() < k) {
        cerr << "Error: Not enough points in the JSON file. Found " << points.size() << ", but need " << k << "." << endl;
        return 1;
    }

    try {
        long double secret_double = find_secret(points);
        // The final secret is expected to be a standard integer, so we round and cast.
        long long final_secret = static_cast<long long>(round(secret_double));
        
        cout << "\n----------------------------------" << endl;
        cout << "The calculated secret (C) is: " << final_secret << endl;
        cout << "----------------------------------" << endl;

    } catch (const runtime_error& e) {
        cerr << "An error occurred during calculation: " << e.what() << endl;
        return 1;
    }

    return 0;
}
