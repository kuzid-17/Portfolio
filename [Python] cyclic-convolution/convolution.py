from field import GF2m

def _is_binary(matrix: list) -> bool:
    return all(c in (0, 1) for row in matrix for c in row)

def mat_vec_mul(gf: GF2m, matrix: list, vec: list, binary: bool = False) -> list:
    rows = len(matrix)
    cols = len(matrix[0])
    res = [0] * rows
    for i in range(rows):
        for j in range(cols):
            c = matrix[i][j]
            if c == 0:
                continue
            v = vec[j]
            term = v if c == 1 else gf.mul(c, v)
            if binary or term == 0:
                res[i] ^= term
            else:
                res[i] = gf.add(res[i], term)
    return res

def build_circulant(circulant_row: list) -> list:
    n = len(circulant_row)
    return [[circulant_row[(j + i) % n] for j in range(n)] for i in range(n)]

def naive_convolution(gf: GF2m, circulant_row: list, vec: list) -> list:
    return mat_vec_mul(gf, build_circulant(circulant_row), vec)

def fast_convolution(gf: GF2m, decomposition: dict, vec: list) -> list:
    result = vec[:]
    for mat in decomposition['matrices']:
        result = mat_vec_mul(gf, mat, result, binary=_is_binary(mat))
    return result
