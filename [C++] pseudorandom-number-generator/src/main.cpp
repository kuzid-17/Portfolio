#include "bbs.h"
#include "bg_crypto.h"
#include "fips_tests.h"
#include "primes.h"
#include <cmath>
#include <iostream>

static void print_bits(const std::vector<int>& bits, size_t h = 0) {
    for (size_t i = 0; i < bits.size(); i++) {
        if (h > 0 && i > 0 && i % h == 0) std::cout << ' ';
        std::cout << bits[i];
    }
}

static void run_bbs_fips() {
    std::cout << "=================================================\n";
    std::cout << " BBS PRNG + FIPS 140-1 Statistical Tests\n";
    std::cout << "=================================================\n";

    mpz_t p, q, n, seed;
    mpz_init(p); mpz_init(q); mpz_init(n); mpz_init(seed);

    std::cout << "Generating Blum primes (256 bits each)...\n";
    generate_blum_primes(p, q, 256);
    mpz_mul(n, p, q);

    generate_random_number(seed, 256);
    if (!is_coprime(seed, n)) {
        std::cout << "Seed not coprime with n, adjusting...\n";
        do {
            mpz_add_ui(seed, seed, 1);
        } while (!is_coprime(seed, n));
    }

    BBS gen(n, seed);
    std::cout << "Generating 20000 bits...\n\n";
    std::vector<int> bits = gen.generate_bits(20000);

    std::cout << "First 32 bits: ";
    for (int i = 0; i < 32; i++) std::cout << bits[i];
    std::cout << "\n\n";

    FipsResult res = run_all_fips_tests(bits);
    std::cout << "FIPS 140-1 Test Results:\n";
    std::cout << "------------------------\n";
    std::cout << "Monobit test:  " << (res.monobit   ? "PASS" : "FAIL") << "\n";
    std::cout << "Poker test:    " << (res.poker     ? "PASS" : "FAIL") << "\n";
    std::cout << "Runs test:     " << (res.runs      ? "PASS" : "FAIL") << "\n";
    std::cout << "Long run test: " << (res.long_run  ? "PASS" : "FAIL") << "\n";

    if (res.monobit && res.poker && res.runs && res.long_run)
        std::cout << "\n[OK] all tests passed\n";
    else
        std::cout << "\n[!!] some tests failed\n";

    mpz_clear(p); mpz_clear(q); mpz_clear(n); mpz_clear(seed);
}

static void run_book_example() {
    std::cout << "\n\n\n=================================================\n";
    std::cout << " Book Example 8.57 Verification\n";
    std::cout << "=================================================\n";

    mpz_t p, q;
    mpz_init(p); mpz_set_ui(p, 499);
    mpz_init(q); mpz_set_ui(q, 547);

    BGPublicKey  pub;
    BGPrivateKey priv;
    bg_keygen(pub, priv, p, q);
    mpz_clear(p); mpz_clear(q);

    std::vector<int> plain = {1,0,0,1, 1,1,0,0, 0,0,0,1, 0,0,0,0, 1,1,0,0};

    mpz_t r;
    mpz_init(r); mpz_set_ui(r, 399);
    BGCiphertext ct = bg_encrypt(pub, plain, r);
    mpz_clear(r);

    std::vector<std::vector<int>> expected_c = {
        {0,0,1,0}, {0,0,0,0}, {1,1,0,0}, {1,1,1,0}, {0,1,0,0}
    };
    long expected_xfinal = 139680;

    bool enc_ok = true;
    std::cout << "Ciphertext blocks:\n";
    for (size_t i = 0; i < ct.blocks.size(); i++) {
        bool match = (ct.blocks[i] == expected_c[i]);
        if (!match) {
            enc_ok = false;
        }

        std::cout << "  c" << i + 1 << ": ";

        for (int b : ct.blocks[i]) {
            std::cout << b;
        }

        std::cout << "  (expected: ";

        for (int b : expected_c[i]) {
            std::cout << b;
        }

        std::cout << ")  " << (match ? "OK" : "MISMATCH") << "\n";
    }
    bool xf_ok = (mpz_get_ui(ct.x_final) == (unsigned long)expected_xfinal);
    gmp_printf("  x_final: %Zd (expected %ld)  %s\n", ct.x_final, expected_xfinal, xf_ok ? "OK" : "MISMATCH");

    size_t t = ct.blocks.size();
    std::vector<int> recovered = bg_decrypt(priv, pub, ct, t);

    bool dec_ok = (recovered == plain);

    std::cout << "Decrypted bits: "; print_bits(recovered, 4);
    std::cout << "\nOriginal bits:  "; print_bits(plain, 4);

    if (enc_ok && xf_ok && dec_ok) {
        std::cout << "\n\n[OK] all book example values verified\n";
    } else {
        std::cout << "\n\n[!!] some values do not match the book\n";
    }

    bg_free_public(pub);
    bg_free_private(priv);
    bg_free_ciphertext(ct);
}

static void run_blum_goldwasser() {
    std::cout << "\n\n\n=================================================\n";
    std::cout << " Blum-Goldwasser Probabilistic Encryption\n";
    std::cout << "=================================================\n";

    mpz_t p, q;
    mpz_init(p); mpz_init(q);
    std::cout << "Generating Blum primes (512 bits each)...\n";
    generate_blum_primes(p, q, 512);

    BGPublicKey  pub;
    BGPrivateKey priv;
    bg_keygen(pub, priv, p, q);
    mpz_clear(p); mpz_clear(q);

    std::cout << "Public key n: " << mpz_sizeinbase(pub.n, 2) << " bits\n";

    std::string message = "Blum-Goldwasser encryption works!";
    std::cout << "\nPlaintext:  \"" << message << "\"\n";

    std::vector<int> plain_bits = string_to_bits(message);

    mpz_t r;
    mpz_init(r);
    generate_random_number(r, 512);
    mpz_mod(r, r, pub.n);
    while (!is_coprime(r, pub.n)) mpz_add_ui(r, r, 1);

    BGCiphertext ct = bg_encrypt(pub, plain_bits, r);
    mpz_clear(r);

    gmp_printf("x_final:     %Zd\n", ct.x_final);

    std::vector<int> recovered_bits = bg_decrypt(priv, pub, ct, ct.blocks.size());
    recovered_bits.resize(plain_bits.size());

    std::string recovered = bits_to_string(recovered_bits);
    std::cout << "Decrypted:  \"" << recovered << "\"\n";
    std::cout << (recovered == message ? "\n[OK] encryption/decryption successful\n" : "\n[!!] mismatch — check implementation\n");

    bg_free_public(pub);
    bg_free_private(priv);
    bg_free_ciphertext(ct);
}

int main() {
    run_bbs_fips();
    run_book_example();
    run_blum_goldwasser();
    
    return 0;
}
