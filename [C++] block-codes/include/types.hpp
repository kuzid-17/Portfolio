#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <complex>

typedef std::uint8_t bit;
typedef std::vector<bit> bitvec;
typedef std::array<bit, 20> codebits;
typedef std::array<double, 20> softbits;
typedef std::complex<double> point;
typedef std::array<point, 10> signal;
