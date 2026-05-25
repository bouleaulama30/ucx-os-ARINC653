#include "randombytes.h"
#include <stddef.h>
#include <stdint.h>

#ifdef RANDOM_TRNG

#include "trng.h"

int randombytes(uint8_t *output, size_t n)
{
    static bool initialized = false;
    static uint32_t word = 0u;
    static unsigned bytes_left = 0u;

    if (output == NULL) {
        return -1;
    }

    if (!initialized) {
        trng_init();
        initialized = true;
    }

    while (n > 0u) {
        if (bytes_left == 0u) {
            while (!trng_ready()) {
                if (trng_failed()) {
                    return -2;
                }
            }

            word = trng_read_word();
            bytes_left = 4u;
        }

        *output++ = (uint8_t)(word & 0xFFu);
        word >>= 8;
        bytes_left--;
        n--;
    }

    return 0;
}




#else

int randombytes(uint8_t *output, size_t n) {
    static uint32_t x = 0x12345678u;

    for (size_t i = 0; i < n; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        output[i] = (uint8_t)x;
    }
    return 0;
}

#endif
