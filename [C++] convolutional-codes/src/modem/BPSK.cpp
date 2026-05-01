#include "modem.hpp"

std::vector<double> BPSKmod(const std::vector<int>& input_bits) {
    size_t length = input_bits.size();
    std::vector<double> modulated_signal(length);

    for (size_t i = 0; i < length; i++) {
        modulated_signal[i] = 1 - 2 * input_bits[i];
    }

    return modulated_signal;
}

std::vector<int> BPSKdemod(const std::vector<double>& input_signal) {
    size_t length = input_signal.size();
    std::vector<int> output_bits(length);

    for (size_t i = 0; i < length; i++) {
        if (input_signal[i] < 0) {
            output_bits[i] = 1;
        } else {
            output_bits[i] = 0;
        }
    }

    return output_bits;
}

std::vector<double> BPSKdemod_LLR(const std::vector<double>& input_signal, double SNR, double R) {
    double SNR_linear = std::pow(10.0, SNR/10.0);
    double noise_variance = 1.0 / (2.0 * SNR_linear * R);

    std::vector<double> llr(input_signal.size());

    for (size_t i = 0; i < input_signal.size(); i++) {
        llr[i] = 2.0 * input_signal[i] / noise_variance;
    }

    return llr;
}

std::vector<double> BPSKdemod_LLR_Rayleigh(const std::vector<double>& input_signal, const std::vector<double>& fading_mu, double SNR, double R) {
    size_t length = input_signal.size();
    std::vector<double> llr(length);
    
    double SNR_lin = std::pow(10.0, SNR / 10.0);
    double noise_variance = 1.0 / (2.0 * SNR_lin * R);
    
    for (size_t i = 0; i < length; i++) {
        llr[i] = 2.0 * fading_mu[i] * input_signal[i] / noise_variance;
    }
    
    return llr;
}
