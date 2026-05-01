#include "fips_tests.h"
#include <cmath>
#include <iostream>

bool monobit_test(const std::vector<int>& bits) {
    int ones = 0;
    for (int b : bits) {
        if (b == 1) {
            ones++;
        }
    }

    return (ones > 9654 && ones < 10346);
}

bool poker_test(const std::vector<int>& bits, size_t m) {
    if (m != 4) {
        return true;
    }
    
    size_t k = bits.size() / m;
    std::vector<int> freq(1 << m, 0);

    for (size_t i = 0; i < k; i++) {
        int value = 0;
        for (size_t j = 0; j < m; j++) {
            value = (value << 1) | bits[i * m + j];
        }
        freq[value]++;
    }

    double sum = 0.0;
    for (int f : freq) {
        sum += (double)f * f;
    }
    double x = (16.0 / k) * sum - k;

    return (x > 1.03 && x < 57.4);
}

bool runs_test(const std::vector<int>& bits) {
    int runs[6][2] = {0};

    int current = bits[0];
    int length = 1;

    for (size_t i = 1; i < bits.size(); i++) {
        if (bits[i] == current) {
            length++;
        } else {
            int idx = length - 1;
            if (idx > 5) {
                idx = 5;
            }
            runs[idx][current]++;

            current = bits[i];
            length = 1;
        }
    }

    int idx = length - 1;
    if (idx > 5) {
        idx = 5;
    }
    runs[idx][current]++;

    const int limits[6][2] = {
        {2267, 2733},
        {1079, 1421},
        {502, 748},
        {223, 402},
        {90, 223},
        {90, 223}
    };

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 2; j++) {
            if (runs[i][j] < limits[i][0] || runs[i][j] > limits[i][1]) {
                return false;
            }
        }
    }

    return true;
}

bool long_run_test(const std::vector<int>& bits) {
    int max_run = 1;
    int current_run = 1;

    for (size_t i = 1; i < bits.size(); i++) {
        if (bits[i] == bits[i - 1]) {
            current_run++;
            if (current_run > max_run) {
                max_run = current_run;
            }
        } else {
            current_run = 1;
        }
    }

    return max_run < 34;
}

FipsResult run_all_fips_tests(const std::vector<int>& bits) {
    FipsResult res;

    if (bits.size() != 20000) {
        std::cerr << "Warning: FIPS tests require 20000 bits, got " << bits.size() << std::endl;
    }
    res.monobit = monobit_test(bits);
    res.poker = poker_test(bits, 4);
    res.runs = runs_test(bits);
    res.long_run = long_run_test(bits);

    return res;
}
