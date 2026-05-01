#pragma once

#include "types.hpp"

bitvec generateRandomBits(size_t size);
double calculateBLER(size_t block_size, bitvec& input_data, bitvec& decoded_data);
