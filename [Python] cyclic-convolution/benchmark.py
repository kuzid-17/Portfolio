import random
from field import GF2m
from convolution import naive_convolution, fast_convolution

def make_counter():
    return {'mul': 0, 'add': 0}

def random_vector(size: int, max_val: int) -> list:
    return [random.randint(0, max_val) for _ in range(size)]

def count_ops(m: int, primitive_poly: int, decomposition_func, num_tests: int = 200):
    max_val = (1 << m) - 1
    total_naive = make_counter()
    total_fast  = make_counter()

    for _ in range(num_tests):
        vec = random_vector(m, max_val)

        cnt = make_counter()
        gf  = GF2m(m, primitive_poly, counter=cnt)
        dec = decomposition_func(gf)
        naive_convolution(gf, dec['circulant_row'], vec)
        total_naive['mul'] += cnt['mul']
        total_naive['add'] += cnt['add']

        cnt = make_counter()
        gf  = GF2m(m, primitive_poly, counter=cnt)
        dec = decomposition_func(gf)
        fast_convolution(gf, dec, vec)
        total_fast['mul'] += cnt['mul']
        total_fast['add'] += cnt['add']

    avg = lambda d: {k: d[k] / num_tests for k in d}
    return avg(total_naive), avg(total_fast)


def run_benchmark(m: int, primitive_poly: int, decomposition_func, num_tests: int = 200):
    gf  = GF2m(m, primitive_poly)
    dec = decomposition_func(gf)
    vec = random_vector(m, (1 << m) - 1)
    naive_res = naive_convolution(gf, dec['circulant_row'], vec)
    fast_res  = fast_convolution(gf, dec, vec)

    W = 50
    print(f"\n{''*W}")

    superscript_map = str.maketrans("468", "⁴⁶⁸")
    m_sup = str(m).translate(superscript_map)

    print(f"GF(2{m_sup})".center(W))
    print(f"{'='*W}")
    correct = (naive_res == fast_res)
    print(f"  Корректность: {'OK' if correct else 'ОШИБКА!'}")
    if not correct:
        print(f"  Наивный: {naive_res}")
        print(f"  Быстрый: {fast_res}")
        return

    naive_ops, fast_ops = count_ops(m, primitive_poly, decomposition_func, num_tests)

    mul_speedup = naive_ops['mul'] / fast_ops['mul'] if fast_ops['mul'] else float('inf')
    add_speedup = naive_ops['add'] / fast_ops['add'] if fast_ops['add'] else float('inf')

    print(f"  {'Метод':<16} {'Mult':>14} {'Add':>14}")
    print(f"  {'-'*46}")
    print(f"  {'Наивный':<16} {naive_ops['mul']:>14.1f} {naive_ops['add']:>14.1f}")
    print(f"  {'Быстрый':<16} {fast_ops['mul']:>14.1f} {fast_ops['add']:>14.1f}")
    print(f"  {'-'*46}")
    print(f"  {'Выигрыш':<16} {mul_speedup:>13.2f}x {add_speedup:>13.2f}x")
    print(f"{''*W}\n")
    