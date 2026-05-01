#include "bbs.h"

BBS::BBS(const mpz_t n_in, const mpz_t seed) {
    mpz_init(n);
    mpz_init(state);
    
    mpz_set(n, n_in);
    
    mpz_t gcd_result;
    mpz_init(gcd_result);
    mpz_gcd(gcd_result, seed, n);
    
    if (mpz_cmp_ui(gcd_result, 1) != 0) {
        mpz_t temp;
        mpz_init(temp);
        mpz_set(temp, seed);
        
        do {
            mpz_add_ui(temp, temp, 1);
            mpz_gcd(gcd_result, temp, n);
        } while (mpz_cmp_ui(gcd_result, 1) != 0);

        mpz_powm_ui(state, temp, 2, n);
        mpz_clear(temp);
    } else {
        mpz_powm_ui(state, seed, 2, n);
    }
    
    mpz_clear(gcd_result);
}

BBS::~BBS() {
    mpz_clear(n);
    mpz_clear(state);
}

int BBS::next_bit() {
    mpz_powm_ui(state, state, 2, n);
    return mpz_tstbit(state, 0);
}

std::vector<int> BBS::generate_bits(size_t nbits) {
    std::vector<int> bits;
    bits.reserve(nbits);
    
    for (size_t i = 0; i < nbits; i++) {
        bits.push_back(next_bit());
    }
    
    return bits;
}

std::vector<unsigned char> BBS::generate_bytes(size_t nbytes) {
    std::vector<unsigned char> bytes;
    bytes.reserve(nbytes);
    
    for (size_t i = 0; i < nbytes; i++) {
        unsigned char b = 0;
        for (int j = 0; j < 8; j++) {
            b = (b << 1) | next_bit();
        }
        bytes.push_back(b);
    }
    
    return bytes;
}

void BBS::get_state(mpz_t result) const {
    mpz_set(result, state);
}

void BBS::get_modulus(mpz_t result) const {
    mpz_set(result, n);
}
