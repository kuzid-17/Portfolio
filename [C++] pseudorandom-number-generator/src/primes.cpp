#include "primes.h"
#include <gmp.h>
#include <ctime>

gmp_randstate_t state;

void init_rand() {
    static bool init = false;

    if (!init) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));

        init = true;
    }
}

void generate_random_number(mpz_t result, size_t bits) {
    init_rand();
    mpz_urandomb(result, state, bits);
}

bool is_probable_prime(const mpz_t n, int iter) {
    return mpz_probab_prime_p(n, iter) > 0;
}

bool is_blum_prime(const mpz_t p) {
    return mpz_fdiv_ui(p, 4) == 3;
}

void generate_blum_prime(mpz_t result, size_t bits) {
    mpz_t tmp;
    mpz_init(tmp);

    while (true) {
        generate_random_number(tmp, bits);
        mpz_setbit(tmp, bits - 1);
        mpz_setbit(tmp, 0);
        mpz_setbit(tmp, 1);

        if (mpz_fdiv_ui(tmp, 4) != 3) {
            continue;
        }

        if (is_probable_prime(tmp, 25)) {
            mpz_set(result, tmp);
            break;
        }
    }

    mpz_clear(tmp);
}

void generate_blum_primes(mpz_t p, mpz_t q, size_t bits) {
    generate_blum_prime(p, bits);
    do {
        generate_blum_prime(q, bits);
    } while (mpz_cmp(p, q) == 0);
}

bool is_coprime(const mpz_t a, const mpz_t b) {
    mpz_t g;
    mpz_init(g);
    mpz_gcd(g, a, b);
    bool res = (mpz_cmp_ui(g, 1) == 0);

    mpz_clear(g);
    return res;
}
