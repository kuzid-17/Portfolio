#pragma once

#include "types.hpp"
#include <iostream>
#include <string>

struct InputData {
    std::string mode;
    size_t num_of_pucch_f2_bits;

    bitvec pucch_f2_bits;

    signal qpsk_symbols;

    size_t iterations;
    double noise_level;
};

InputData parseInput(const std::string& filename);
void throwError(const std::string& message);

void writeOutputCoding(const signal& symbols);
void writeOutputDecoding(size_t num_bits, const bitvec& bits);
void writeOutputSimulation(size_t num_bits, double bler, size_t success, size_t failed);

std::string complexToString(const point& p);
point stringToComplex(const std::string& s);
