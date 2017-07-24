#include <stddef.h> // for size_t

#ifndef SEEK_SET
#define SEEK_SET (0)
#define SEEK_CUR (1)
#define SEEK_END (2)
#endif

#define ssize_t int
ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, const void *buf, size_t nbyte);
#undef ssize_t
