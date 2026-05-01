#pragma once

#include "types.hpp"

class GeneratorMatrix {
public:
    static const size_t ROWS = 20;
    static const size_t COLS = 13;

    static const bit G[ROWS][COLS];

    codebits operator*(const bitvec& data) const;
};

class BlockCode {
public:
    explicit BlockCode(size_t n);

    codebits encode(const bitvec& data) const;
    bitvec decode(const softbits& LLR) const;

private:
    size_t n_;
    GeneratorMatrix gm_;
};
