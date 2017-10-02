#define pgm_read_byte(x) *(x)
#define pgm_read_byte_near(x) *(x)
#define pgm_read_ptr(x) *(x)
#define pgm_read_word(x) *(x)

#define byte unsigned char

#include <string.h>

#define memcpy_P(d,s,n) memcpy(d, s, n)
