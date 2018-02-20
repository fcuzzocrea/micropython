#include <stdint.h>
#include <math.h>

static const double toint = 4503599627370496.0; // 2**52

double nearbyint(double x) {
    union { double f; uint64_t i; } u = {x};
    int e = u.i >> 52 & 0x7ff;
    int s = u.i >> 63;

    if (e >= 0x3ff + 52) {
        // Large enough that it doesn't have a fraction
    } else if (s) {
        x -= toint;
        x += toint;
        if (x == 0) {
            x = -0.0;
        }
    } else {
        x += toint;
        x -= toint;
    }

    return x;
}
