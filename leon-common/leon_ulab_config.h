// Define macros/functions available in newer versions of MicroPython
// that are not in the LEON version.
#define MP_ROM_NONE MP_ROM_PTR(&mp_const_none_obj)
#define MP_ROM_FALSE MP_ROM_PTR(&mp_const_false_obj)
#define MP_ROM_TRUE MP_ROM_PTR(&mp_const_true_obj)
#define MP_ERROR_TEXT(t) t
#define mp_obj_is_type MP_OBJ_IS_TYPE
#define mp_obj_is_int MP_OBJ_IS_INT
#define MP_REGISTER_MODULE(a, b, c)

void mp_obj_slice_indices(mp_obj_t self_in, mp_int_t length, mp_bound_slice_t *result);

// Configure ulab.
#define ULAB_MAX_DIMS                       4
#define ULAB_NUMPY_HAS_LOAD                 (0)
#define ULAB_NUMPY_HAS_LOADTXT              (0)
#define ULAB_NUMPY_HAS_SAVE                 (0)
#define ULAB_NUMPY_HAS_SAVETXT              (0)
#define ULAB_SCIPY_SPECIAL_HAS_GAMMALN      (0) // RTEMS_6_GR740 doesn't link, it has undefined reference to __getreent

// RTEMS 4.8 doesn't provide some declarations or typedefs.
#if RTEMS_4_8
#undef log2
double round(double);
double log2(double);
typedef int ssize_t;
#endif

// RTEMS 6 requires some math function workarounds.
#if RTEMS_6
#define ULAB_TGAMMA_CHECK_DOMAIN (1)
#endif
