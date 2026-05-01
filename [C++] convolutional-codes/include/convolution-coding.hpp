#include "universal-trellis.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <cmath>
#include <climits>

typedef struct {
    uint8_t numInputSymbols = 2;
    uint8_t numOutputSymbols = 4;
    uint8_t numStates = 4;

    uint8_t nextStates[4][2] = {
        {0,2},
        {0,2},
        {1,3},
        {1,3}
    };

    uint8_t outputs[4][2] = {
        {0,3},
        {3,0},
        {2,1},
        {1,2}
    };
    
} trellis75;

std::vector<int> hard_viterbi_decode(const Trellis &T, std::vector<int>& encoded);
std::vector<int> soft_viterbi_decode(const Trellis &T, std::vector<double>& encoded);

std::vector<int> tail_biting_encode(const Trellis& T, std::vector<int>& input_bits);
std::vector<int> hard_wava_decode(const Trellis &T, std::vector<int>& encoded, int maxIter);
std::vector<int> soft_wava_decode(const Trellis &T, std::vector<double>& encoded, int maxIter);

std::vector<std::vector<int>> hard_parallel_lva(const Trellis &T, std::vector<int>& encoded, int L);
std::vector<std::vector<int>> soft_parallel_lva(const Trellis &T, std::vector<double>& encoded, int L);
