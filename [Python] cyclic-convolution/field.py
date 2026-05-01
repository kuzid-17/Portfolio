class GF2m:
    def __init__(self, m: int, primitive_poly: int, counter=None):
        self.m = m
        self.poly = primitive_poly
        self.mask = (1 << m) - 1
        self.counter = counter

    def add(self, a: int, b: int) -> int:
        if self.counter is not None:
            self.counter['add'] += 1
        return a ^ b

    def mul(self, a: int, b: int) -> int:
        if self.counter is not None:
            self.counter['mul'] += 1
        res = 0
        while b:
            if b & 1:
                res ^= a
            a <<= 1
            if a & (1 << self.m):
                a ^= self.poly
            b >>= 1
        return res & self.mask

    def pow(self, a: int, exp: int) -> int:
        res = 1
        while exp > 0:
            if exp & 1:
                res = self._mul_raw(res, a)
            a = self._mul_raw(a, a)
            exp >>= 1
        return res

    def _mul_raw(self, a: int, b: int) -> int:
        res = 0
        while b:
            if b & 1:
                res ^= a
            a <<= 1
            if a & (1 << self.m):
                a ^= self.poly
            b >>= 1
        return res & self.mask
    