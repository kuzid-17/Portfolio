#include "channel.hpp"

#include <random>

signal AWGNchannel(double q, const signal& data) {
    static std::mt19937 gen(std::random_device{}());
    static std::normal_distribution<double> dist(0.0, 1.0);

    signal noised = data;
    for (point& p: noised) {
        p.real(p.real() + dist(gen) / q);
        p.imag(p.imag() + dist(gen) / q);
    }

    return noised;
}
