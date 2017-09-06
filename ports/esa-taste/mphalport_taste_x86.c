#include <unistd.h>

#include "py/mpconfig.h"
#include "py/mphal.h"

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    int ret = write(1, str, len);
    (void)ret;
}

// Send "cooked" string of length, where LF is replaced with CR LF.
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    mp_hal_stdout_tx_strn(str, len);
}
