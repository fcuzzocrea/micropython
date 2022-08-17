#include <stdlib.h>

int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

size_t strlen(const char *s);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);