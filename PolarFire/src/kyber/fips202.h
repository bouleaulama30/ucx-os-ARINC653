#ifndef FIPS202_H
#define FIPS202_H

#include <stddef.h>
#include <stdint.h>


#define SHAKE128_RATE 168
#define SHAKE256_RATE 136
#define SHA3_256_RATE 136
#define SHA3_512_RATE   72

typedef struct {
    uint64_t s[25];
} shake128ctx;

typedef struct {
    uint64_t s[25];
    size_t pos;
    uint8_t squeezing;
} shake256incctx;

void shake128_absorb(shake128ctx *state, const uint8_t *input, size_t inlen);
void shake128_squeezeblocks(uint8_t *output, size_t nblocks, shake128ctx *state);
void shake128_ctx_release(shake128ctx *state);

void shake256_inc_init(shake256incctx *state);
void shake256_inc_absorb(shake256incctx *state, const uint8_t *input, size_t inlen);
void shake256_inc_finalize(shake256incctx *state);
void shake256_inc_squeeze(uint8_t *output, size_t outlen, shake256incctx *state);
void shake256_inc_ctx_release(shake256incctx *state);

void shake256(uint8_t *output, size_t outlen, const uint8_t *input, size_t inlen);

void sha3_256(uint8_t *output, const uint8_t *input, size_t inlen);
void sha3_512(uint8_t *output, const uint8_t *input, size_t inlen);

#endif
