#include "simulation.hpp"
#include <random>

bitvec generateRandomBits(size_t size) {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 1);
    
    bitvec random_vector(size);
    for (bit& b : random_vector) {
        b = dist(gen);
    }

    return random_vector;
}

double calculateBLER(size_t block_size, const bitvec& input_data, const bitvec& decoded_data) {
    size_t error_blocks = 0;
    size_t total_blocks = input_data.size() / block_size;

    for (size_t b = 0; b < total_blocks; b++) {
        for (size_t i = 0; i < block_size; i++) {
            if (input_data[b * block_size + i] != decoded_data[b * block_size + i]) {
                error_blocks++;
                break;
            }
        }
    }

    return double(error_blocks) / total_blocks;
}
