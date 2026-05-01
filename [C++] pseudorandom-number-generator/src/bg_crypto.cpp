#include "bg_crypto.h"
#include <gmp.h>
#include <cmath>
#include <stdexcept>
#include <iostream>

std::vector<int> string_to_bits(const std::string& s) {
    std::vector<int> bits;
    bits.reserve(s.size() * 8);

    for (unsigned char c : s) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }

    return bits;
}

std::string bits_to_string(const std::vector<int>& bits) {
    std::string s;
    size_t full = bits.size() / 8;

    for (size_t i = 0; i < full; ++i) {
        unsigned char c = 0;
        for (int j = 0; j < 8; ++j) {
            c = (c << 1) | bits[i * 8 + j];
        }
        s += (char)c;
    }

    return s;
}

void bg_keygen(BGPublicKey& pub, BGPrivateKey& priv, const mpz_t p_in, const mpz_t q_in) {
    mpz_init(pub.n);
    mpz_init(priv.p); mpz_init(priv.q);
    mpz_init(priv.a); mpz_init(priv.b);

    mpz_set(priv.p, p_in);
    mpz_set(priv.q, q_in);

    mpz_mul(pub.n, priv.p, priv.q);

    mpz_t g;
    mpz_init(g);
    mpz_gcdext(g, priv.a, priv.b, priv.p, priv.q);

    if (mpz_cmp_ui(g, 1) != 0) {
        mpz_clear(g);
        throw std::runtime_error("bg_keygen: p and q are not coprime");
    }

    mpz_clear(g);
}

BGCiphertext bg_encrypt(const BGPublicKey& pub, const std::vector<int>& plaintext_bits, const mpz_t r_seed) {
    size_t k = mpz_sizeinbase(pub.n, 2) - 1;
    size_t h = (size_t)std::floor(std::log2((double)k));

    std::vector<int> padded = plaintext_bits;
    while (padded.size() % h != 0) {
        padded.push_back(0);
    }
    size_t t = padded.size() / h;

    BGCiphertext ct;
    ct.blocks.resize(t, std::vector<int>(h));
    mpz_init(ct.x_final);

    mpz_t x;
    mpz_init(x);
    mpz_powm_ui(x, r_seed, 2, pub.n);

    for (size_t i = 0; i < t; ++i) {
        mpz_powm_ui(x, x, 2, pub.n);

        std::vector<int> pi(h);
        for (size_t bit = 0; bit < h; ++bit) {
            pi[h - 1 - bit] = (int)mpz_tstbit(x, bit);
        }

        for (size_t bit = 0; bit < h; ++bit) {
            ct.blocks[i][bit] = pi[bit] ^ padded[i * h + bit];
        }
    }
    mpz_powm_ui(ct.x_final, x, 2, pub.n);

    mpz_clear(x);
    return ct;
}

std::vector<int> bg_decrypt(const BGPrivateKey& priv, const BGPublicKey& pub, const BGCiphertext& ct, size_t t) {
    size_t k = mpz_sizeinbase(pub.n, 2) - 1;
    size_t h = (size_t)std::floor(std::log2((double)k));

    mpz_t d1, d2, u, v, x0, x;
    mpz_t pm1, qm1, base1, base2, exp_t1;
    mpz_init(d1); mpz_init(d2);
    mpz_init(u);  mpz_init(v);
    mpz_init(x0); mpz_init(x);
    mpz_init(pm1); mpz_init(qm1);
    mpz_init(base1); mpz_init(base2);
    mpz_init(exp_t1);

    mpz_sub_ui(pm1, priv.p, 1);
    mpz_sub_ui(qm1, priv.q, 1);

    mpz_add_ui(base1, priv.p, 1);
    mpz_divexact_ui(base1, base1, 4);

    mpz_add_ui(base2, priv.q, 1);
    mpz_divexact_ui(base2, base2, 4);

    mpz_set_ui(exp_t1, (unsigned long)(t + 1));
    mpz_powm(d1, base1, exp_t1, pm1);
    mpz_powm(d2, base2, exp_t1, qm1);
    mpz_powm(u, ct.x_final, d1, priv.p);
    mpz_powm(v, ct.x_final, d2, priv.q);

    mpz_t term1, term2;
    mpz_init(term1); mpz_init(term2);

    mpz_mul(term1, v, priv.a);
    mpz_mul(term1, term1, priv.p);

    mpz_mul(term2, u, priv.b);
    mpz_mul(term2, term2, priv.q);

    mpz_add(x0, term1, term2);
    mpz_mod(x0, x0, pub.n);

    mpz_clear(term1); mpz_clear(term2);

    mpz_set(x, x0);
    std::vector<int> plaintext_bits;
    plaintext_bits.reserve(t * h);

    for (size_t i = 0; i < t; ++i) {
        mpz_powm_ui(x, x, 2, pub.n);

        std::vector<int> pi(h);
        for (size_t bit = 0; bit < h; ++bit) {
            pi[h - 1 - bit] = (int)mpz_tstbit(x, bit);
        }

        for (size_t bit = 0; bit < h; ++bit) {
            plaintext_bits.push_back(pi[bit] ^ ct.blocks[i][bit]);
        }
    }

    mpz_clear(d1); mpz_clear(d2);
    mpz_clear(u);  mpz_clear(v);
    mpz_clear(x0); mpz_clear(x);
    mpz_clear(pm1); mpz_clear(qm1);
    mpz_clear(base1); mpz_clear(base2);
    mpz_clear(exp_t1);

    return plaintext_bits;
}

void bg_free_public(BGPublicKey& pub) {
    mpz_clear(pub.n);
}

void bg_free_private(BGPrivateKey& priv) {
    mpz_clear(priv.p); mpz_clear(priv.q);
    mpz_clear(priv.a); mpz_clear(priv.b);
}

void bg_free_ciphertext(BGCiphertext& ct) {
    mpz_clear(ct.x_final);
}
