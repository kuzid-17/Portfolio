#ifndef PRIMES_H
#define PRIMES_H

#include <gmp.h>
#include <cstddef>

void generate_random_number(mpz_t result, size_t bits);
bool is_probable_prime(const mpz_t n, int iterations);

void generate_blum_prime(mpz_t result, size_t bits);
void generate_blum_primes(mpz_t p, mpz_t q, size_t bits);

bool is_coprime(const mpz_t a, const mpz_t b);
bool is_blum_prime(const mpz_t p);

#endif // PRIMES_H
