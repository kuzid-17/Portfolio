#include "modem.hpp"

signal QPSKmod(const codebits& data) {
    signal modulated;

    for (size_t i = 0; i < modulated.size(); i++) {
        double componentI = (data[2 * i] * 2 - 1) / sqrt(2);
        double componentQ = (data[2 * i + 1] * 2 - 1) / sqrt(2);
        modulated[i] = point(componentI, componentQ);
    }

    return modulated;
}

softbits QPSKdemod(const signal& modulated) {
    softbits demodulated;

    for (size_t i = 0; i < modulated.size(); i++) {
        demodulated[2 * i] = modulated[i].real();
        demodulated[2 * i + 1] = modulated[i].imag();
    }

    return demodulated;
}
