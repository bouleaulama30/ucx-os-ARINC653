#include "fips202.h"

#include <stddef.h>
#include <stdint.h>
#include "libc.h"


static inline uint64_t rol64(uint64_t x, unsigned n) {
    return (n == 0) ? x : ((x << n) | (x >> (64 - n)));
}

static uint64_t load64_le(const uint8_t *x) {
    uint64_t r = 0;
    for (unsigned i = 0; i < 8; i++) {
        r |= (uint64_t)x[i] << (8 * i);
    }
    return r;
}

static void store64_le(uint8_t *x, uint64_t u) {
    for (unsigned i = 0; i < 8; i++) {
        x[i] = (uint8_t)(u >> (8 * i));
    }
}

static void keccakf1600(uint64_t s[25]) {
    static const uint64_t RC[24] = {
        0x0000000000000001ULL, 0x0000000000008082ULL,
        0x800000000000808aULL, 0x8000000080008000ULL,
        0x000000000000808bULL, 0x0000000080000001ULL,
        0x8000000080008081ULL, 0x8000000000008009ULL,
        0x000000000000008aULL, 0x0000000000000088ULL,
        0x0000000080008009ULL, 0x000000008000000aULL,
        0x000000008000808bULL, 0x800000000000008bULL,
        0x8000000000008089ULL, 0x8000000000008003ULL,
        0x8000000000008002ULL, 0x8000000000000080ULL,
        0x000000000000800aULL, 0x800000008000000aULL,
        0x8000000080008081ULL, 0x8000000000008080ULL,
        0x0000000080000001ULL, 0x8000000080008008ULL
    };

    static const unsigned R[24] = {
         1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14,
        27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44
    };

    static const unsigned PI[24] = {
        10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4,
        15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1
    };

    for (unsigned round = 0; round < 24; round++) {
        uint64_t C[5], D[5], B[5];

        /* theta */
        for (unsigned x = 0; x < 5; x++) {
            C[x] = s[x] ^ s[x + 5] ^ s[x + 10] ^ s[x + 15] ^ s[x + 20];
        }
        for (unsigned x = 0; x < 5; x++) {
            D[x] = C[(x + 4) % 5] ^ rol64(C[(x + 1) % 5], 1);
        }
        for (unsigned i = 0; i < 25; i++) {
            s[i] ^= D[i % 5];
        }

        /* rho + pi */
        {
            uint64_t cur = s[1];
            for (unsigned t = 0; t < 24; t++) {
                unsigned j = PI[t];
                uint64_t tmp = s[j];
                s[j] = rol64(cur, R[t]);
                cur = tmp;
            }
        }

        /* chi */
        for (unsigned y = 0; y < 5; y++) {
            for (unsigned x = 0; x < 5; x++) {
                B[x] = s[5 * y + x];
            }
            for (unsigned x = 0; x < 5; x++) {
                s[5 * y + x] = B[x] ^ ((~B[(x + 1) % 5]) & B[(x + 2) % 5]);
            }
        }

        /* iota */
        s[0] ^= RC[round];
    }
}

static void keccak_absorb(uint64_t s[25],
                          unsigned rate,
                          const uint8_t *in,
                          size_t inlen,
                          uint8_t delim) {
    uint8_t last[200];

    ucx_memset(s, 0, 25 * sizeof(uint64_t));

    while (inlen >= rate) {
        for (unsigned i = 0; i < rate / 8; i++) {
            s[i] ^= load64_le(in + 8 * i);
        }
        keccakf1600(s);
        in += rate;
        inlen -= rate;
    }

    ucx_memset(last, 0, sizeof(last));
    ucx_memcpy(last, in, inlen);
    last[inlen] = delim;
    last[rate - 1] |= 0x80;

    for (unsigned i = 0; i < rate / 8; i++) {
        s[i] ^= load64_le(last + 8 * i);
    }
}

static void keccak_squeezeblocks(uint8_t *out,
                                 size_t nblocks,
                                 uint64_t s[25],
                                 unsigned rate) {
    while (nblocks--) {
        keccakf1600(s);
        for (unsigned i = 0; i < rate / 8; i++) {
            store64_le(out + 8 * i, s[i]);
        }
        out += rate;
    }
}

static void keccak_squeeze(uint8_t *out,
                           size_t outlen,
                           uint64_t s[25],
                           unsigned rate,
                           size_t *pos) {
    uint8_t block[200];

    while (outlen > 0) {
        if (*pos == rate) {
            keccakf1600(s);
            *pos = 0;
        }

        for (unsigned i = 0; i < rate / 8; i++) {
            store64_le(block + 8 * i, s[i]);
        }

        size_t take = rate - *pos;
        if (take > outlen) {
            take = outlen;
        }

        ucx_memcpy(out, block + *pos, take);
        out += take;
        outlen -= take;
        *pos += take;
    }
}

void shake128_absorb(shake128ctx *state, const uint8_t *input, size_t inlen) {
    keccak_absorb(state->s, SHAKE128_RATE, input, inlen, 0x1F);
}

void shake128_squeezeblocks(uint8_t *output, size_t nblocks, shake128ctx *state) {
    keccak_squeezeblocks(output, nblocks, state->s, SHAKE128_RATE);
}

void shake128_ctx_release(shake128ctx *state) {
    (void)state;
}

void shake256_inc_init(shake256incctx *state) {
    ucx_memset(state, 0, sizeof(*state));
}

void shake256_inc_absorb(shake256incctx *state, const uint8_t *input, size_t inlen) {
    while (inlen > 0) {
        size_t take = SHAKE256_RATE - state->pos;
        if (take > inlen) {
            take = inlen;
        }

        for (size_t i = 0; i < take; i++) {
            size_t off = state->pos + i;
            state->s[off >> 3] ^= (uint64_t)input[i] << (8 * (off & 7));
        }

        state->pos += take;
        input += take;
        inlen -= take;

        if (state->pos == SHAKE256_RATE) {
            keccakf1600(state->s);
            state->pos = 0;
        }
    }
}

void shake256_inc_finalize(shake256incctx *state) {
    state->s[state->pos >> 3] ^= (uint64_t)0x1F << (8 * (state->pos & 7));
    state->s[(SHAKE256_RATE - 1) >> 3] ^= (uint64_t)0x80 << (8 * ((SHAKE256_RATE - 1) & 7));
    state->squeezing = 1;
    state->pos = SHAKE256_RATE;
}

void shake256_inc_squeeze(uint8_t *output, size_t outlen, shake256incctx *state) {
    (void)state->squeezing; /* kept for clarity / future checks */
    keccak_squeeze(output, outlen, state->s, SHAKE256_RATE, &state->pos);
}

void shake256_inc_ctx_release(shake256incctx *state) {
    (void)state;
}

void shake256(uint8_t *output, size_t outlen, const uint8_t *input, size_t inlen) {
    uint64_t s[25];
    size_t pos = SHAKE256_RATE;

    keccak_absorb(s, SHAKE256_RATE, input, inlen, 0x1F);
    keccak_squeeze(output, outlen, s, SHAKE256_RATE, &pos);
}

void sha3_256(uint8_t *output, const uint8_t *input, size_t inlen) {
    uint64_t s[25];
    size_t pos = SHA3_256_RATE;

    keccak_absorb(s, SHA3_256_RATE, input, inlen, 0x06);
    keccak_squeeze(output, 32, s, SHA3_256_RATE, &pos);
}

void sha3_512(uint8_t *output, const uint8_t *input, size_t inlen) {
    uint64_t s[25];
    size_t pos = SHA3_512_RATE;

    keccak_absorb(s, SHA3_512_RATE, input, inlen, 0x06);
    keccak_squeeze(output, 64, s, SHA3_512_RATE, &pos);
}
