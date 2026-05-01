#include "io.hpp"
#include "codec.hpp"
#include "modem.hpp"
#include "channel.hpp"
#include "simulation.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: missing argument (expected \"filename\".json)\n";
        return 1;
    }

    InputData params = parseInput(argv[1]);
    BlockCode code(params.num_of_pucch_f2_bits);

    if (params.mode == "coding") {
        codebits encoded_data = code.encode(params.pucch_f2_bits);
        signal symbols = QPSKmod(encoded_data);
        writeOutputCoding(symbols);

    } else if (params.mode == "decoding") {
        softbits demodulated_data = QPSKdemod(params.qpsk_symbols);
        bitvec decoded_bits = code.decode(demodulated_data);
        writeOutputDecoding(params.num_of_pucch_f2_bits, decoded_bits);

    } else if (params.mode == "channel simulation") {
        size_t failed = 0;

        for (size_t i = 0; i < params.iterations; i++) {
            bitvec information_bits = generateRandomBits(params.num_of_pucch_f2_bits);
            codebits encoded_data = code.encode(information_bits);
            signal modulated_data = QPSKmod(encoded_data);
            signal noised_data = AWGNchannel(params.noise_level, modulated_data);
            softbits demodulated_data = QPSKdemod(noised_data);
            bitvec decoded_bits = code.decode(demodulated_data);

            for (size_t j = 0; j < params.num_of_pucch_f2_bits; j++) {
                if (information_bits[j] != decoded_bits[j]) {
                    failed++;
                    break;
                }
            }
        }

        double bler = double(failed) / params.iterations;
        writeOutputSimulation(params.num_of_pucch_f2_bits, bler, params.iterations - failed, failed);
    }

    return 0;
}
