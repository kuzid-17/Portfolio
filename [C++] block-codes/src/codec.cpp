#include "codec.hpp"

const bit GeneratorMatrix::G[20][13] = {
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1},
    {1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1},
    {1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1},
    {1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1},
    {1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1},
    {1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1},
    {1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1},
    {1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1},
    {1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0} 
};

codebits GeneratorMatrix::operator*(const bitvec& data) const {
    codebits multiplication_mod2{};
    const size_t n_cols = data.size(); 

    for (size_t i = 0; i < ROWS; i++) {
        bit sum = 0;

        for (size_t j = 0; j < n_cols; j++) {
            sum ^= G[i][j] & data[j];
        }
        multiplication_mod2[i] = sum;
    }
    
    return multiplication_mod2;
}

BlockCode::BlockCode(size_t n) : n_(n) {}

codebits BlockCode::encode(const bitvec& data) const {
    return gm_ * data;
}

bitvec BlockCode::decode(const softbits& LLR) const {
    bitvec decoded(n_);
    double best_metric = -std::numeric_limits<double>::infinity();

    size_t total_combinations = 1u << n_;
    for (size_t mask = 0; mask < total_combinations; mask++) {
        bitvec combination(n_);

        for (size_t i = 0; i < n_; i++) {
            combination[i] = (mask >> i) & 1u;
        }
        codebits codeword = gm_ * combination;

        double metric = 0.0;
        for (size_t i = 0; i < gm_.ROWS; i++) {
            metric += (codeword[i] * 2 - 1) * LLR[i];
        }

        if (metric > best_metric) {
            best_metric = metric;
            decoded = combination;
        }
    }

    return decoded;
}
