#include <stdint.h>
#include <string.h>

// Public Domain implementation of strchr from:
// http://en.wikibooks.org/wiki/C_Programming/Strings#The_strchr_function
char *strchr(const char *s, int c)
{
    /* Scan s for the character.  When this loop is finished,
       s will either point to the end of the string or the
       character we were looking for.  */
    while (*s != '\0' && *s != (char)c)
        s++;
    return ((*s == c) ? (char *) s : 0);
}
