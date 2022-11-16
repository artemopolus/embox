
// #include <assert.h>
// extern "C" void __assert(const char *, int, const char *);

extern "C" void __assert_func(
    const char *file, int line, const char *, const char *e) {
    // __assert(file, line, e);
}
