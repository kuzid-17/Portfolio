from matrices import get_decomposition_m4, get_decomposition_m6, get_decomposition_m8
from benchmark import run_benchmark

def main():
    configs = [
        (4, 0b10011,     get_decomposition_m4),
        (6, 0b1000011,   get_decomposition_m6),
        (8, 0b100011101, get_decomposition_m8),
    ]

    for m, poly, decomp_func in configs:
        run_benchmark(m, poly, decomp_func, num_tests=200)

if __name__ == "__main__":
    main()
