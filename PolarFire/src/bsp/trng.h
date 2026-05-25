#include <stddef.h>
#include <stdint.h>

#ifdef RANDOM_TRNG
#include <stdbool.h>

void trng_init(void);
bool trng_ready(void);
bool trng_failed(void);
uint32_t trng_read_word(void);

#else
int platform_trng_read(uint8_t *out, size_t n);
#endif
