#ifndef FIPS_TESTS_H
#define FIPS_TESTS_H

#include <vector>
#include <cstddef>

struct FipsResult {
    bool monobit;
    bool poker;
    bool runs;
    bool long_run;
};

FipsResult run_all_fips_tests(const std::vector<int>& bits);

bool monobit_test(const std::vector<int>& bits);
bool poker_test(const std::vector<int>& bits, size_t m = 4);
bool runs_test(const std::vector<int>& bits);
bool long_run_test(const std::vector<int>& bits);

#endif // FIPS_TESTS_H
