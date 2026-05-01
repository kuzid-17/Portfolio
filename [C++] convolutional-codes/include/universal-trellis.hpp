#include <vector>
#include <cstdint>

struct Trellis {
    const int k = 1;
    const int numInputSymbols = 2;

    int n = 0;
    int numOutputSymbols = 0;

    int K = 0;
    int numStates = 0;

    std::vector<std::vector<uint32_t>> nextStates;
    std::vector<std::vector<uint32_t>> outputs;
};
