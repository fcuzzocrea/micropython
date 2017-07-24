// Basic floating point support

#define FP_NAN         0
#define FP_INFINITE    1
#define FP_ZERO        2
#define FP_SUBNORMAL   3
#define FP_NORMAL      4

extern double nan(const char *);
extern double pow(double, double);
extern double copysign(double, double);
extern double fmod(double, double);
extern double floor(double);
extern double nearbyint(double);
extern int __fpclassifyd(double x);
extern int __isinfd(double x);
extern int __isnand(double x);
extern int __signbitd(double x);

#define fpclassify(x) __fpclassifyd(x)
#define signbit(x) __signbitd(x)
#define isinf(x) __isinfd(x)
#define isnan(x) __isnand(x)

// Functions needed for math module

extern double sqrt(double);
extern double exp(double);
extern double cos(double);
extern double sin(double);
extern double tan(double);
extern double acos(double);
extern double asin(double);
extern double atan(double);
extern double atan2(double, double);
extern double ceil(double);
extern double fabs(double);
extern double trunc(double);
extern double log(double);
extern double frexp(double, int*);
extern double ldexp(double, int);
extern double modf(double, double*);
static inline double isfinite(double x) {
    int c = fpclassify(x);
    return c != FP_NAN && c != FP_INFINITE;
}

// Functions needed for cmath module

extern double cosh(double);
extern double sinh(double);
