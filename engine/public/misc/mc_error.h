#ifndef MC_MISC_ERROR_H
#define MC_MISC_ERROR_H

#define MC_FAIL_VALUE    0
#define MC_SUCCESS_VALUE 1

#define MC_HAS_SUCCEEDED(predicate) ((predicate)!=MC_FAIL_VALUE)
#define MC_HAS_FAILED   (predicate) ((predicate)==MC_FAIL_VALUE)

#endif // MC_MISC_ERROR_H