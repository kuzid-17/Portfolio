#ifndef BBS_H
#define BBS_H

#include <gmp.h>
#include <vector>
#include <cstddef>

class BBS {
public:
    BBS(const mpz_t n, const mpz_t seed);
    ~BBS();

    int next_bit();
    std::vector<int> generate_bits(size_t n);
    std::vector<unsigned char> generate_bytes(size_t n);

    void get_state(mpz_t result) const;
    void get_modulus(mpz_t result) const;

private:
    mpz_t n;
    mpz_t state;
};

#endif // BBS_H
