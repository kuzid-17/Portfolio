#include "modem.hpp"

double norm2 = 1.0 / std::sqrt(2.0);

std::vector<std::complex<double>> QPSKmod(const std::vector<int>& input_bits) {
    size_t length = input_bits.size() / 2;
    std::vector<std::complex<double>> modulated_signal(length);

    for (size_t i = 0; i < length; i++) {
        modulated_signal[i].real((1 - 2 * input_bits[2 * i]) * norm2);
        modulated_signal[i].imag((1 - 2 * input_bits[2 * i + 1]) * norm2);
    }

    return modulated_signal;
}

std::vector<int> QPSKdemod(const std::vector<std::complex<double>>& input_signal) {
    size_t length = input_signal.size();
    std::vector<int> output_bits(length * 2);

    for (size_t i = 0; i < length; i++) {
        double d00 = std::abs(input_signal[i] - std::complex<double>(1.0 * norm2, 1.0 * norm2));
        double d10 = std::abs(input_signal[i] - std::complex<double>(-1.0 * norm2, 1.0 * norm2));
        double d01 = std::abs(input_signal[i] - std::complex<double>(1.0 * norm2, -1.0 * norm2));
        double d11 = std::abs(input_signal[i] - std::complex<double>(-1.0 * norm2, -1.0 * norm2));

        double min_dist = std::min(std::min(d00, d10), std::min(d01, d11));

        if (min_dist == d00 || min_dist == d01) {
            output_bits[2 * i] = 0;
        } else {
            output_bits[2 * i] = 1;
        }

        if (min_dist == d00 || min_dist == d10) {
            output_bits[2 * i + 1] = 0;
        } else {
            output_bits[2 * i + 1] = 1;
        }
    }

    return output_bits;
}

double calculateLLR(double SNR_dB, double min_dist_difference, double R, double M) {
    double SNR_linear = std::pow(10.0, SNR_dB/10.0);
    double σ² = 1.0 / (2.0 * SNR_linear * R * std::log2(M));
    double result = -1.0 / (2.0 * σ²) * min_dist_difference;
    return result;
}

std::vector<double> QPSKdemod_LLR(const std::vector<std::complex<double>>& input_signal, double SNR, double R) {
    size_t length = input_signal.size();
    std::vector<double> output_bits(length * 2);

    for (size_t i = 0; i < length; i++) {
        double d00 = std::norm(input_signal[i] - std::complex<double>(1.0 * norm2, 1.0 * norm2));
        double d10 = std::norm(input_signal[i] - std::complex<double>(-1.0 * norm2, 1.0 * norm2));
        double d01 = std::norm(input_signal[i] - std::complex<double>(1.0 * norm2, -1.0 * norm2));
        double d11 = std::norm(input_signal[i] - std::complex<double>(-1.0 * norm2, -1.0 * norm2));

        output_bits[2 * i] = calculateLLR(SNR, std::min(d00, d01) - std::min(d10, d11), R, 4);
        output_bits[2 * i + 1] = calculateLLR(SNR, std::min(d00, d10) - std::min(d01, d11), R, 4);
    }

    return output_bits;
}
