#ifndef BG_CRYPTO_H
#define BG_CRYPTO_H

#include <gmp.h>
#include <vector>
#include <string>

struct BGPublicKey {
    mpz_t n;
};

struct BGPrivateKey {
    mpz_t p, q;
    mpz_t a, b;
};

struct BGCiphertext {
    std::vector<std::vector<int>> blocks;
    mpz_t x_final;
};

void bg_keygen(BGPublicKey& pub, BGPrivateKey& priv, const mpz_t p_in, const mpz_t q_in);
BGCiphertext bg_encrypt(const BGPublicKey& pub, const std::vector<int>& plaintext_bits, const mpz_t r_seed);
std::vector<int> bg_decrypt(const BGPrivateKey& priv,const BGPublicKey& pub, const BGCiphertext& ct, size_t t);

void bg_free_public(BGPublicKey& pub);
void bg_free_private(BGPrivateKey& priv);
void bg_free_ciphertext(BGCiphertext& ct);

std::vector<int> string_to_bits(const std::string& s);
std::string bits_to_string(const std::vector<int>& bits);

#endif // BG_CRYPTO_H
