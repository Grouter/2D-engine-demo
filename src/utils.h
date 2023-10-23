#if !defined(UTILS_H)
#define UTILS_H

#define STATIC_ARRAY_CAPACITY(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define kilobytes(value) ((value) * 1024LL)
#define megabytes(value) (kilobytes((value)) * 1024LL)
#define gigabytes(value) (megabytes((value)) * 1024LL)

#define assert_m(expr, msg) assert((msg, expr))
#define invalid_code_path(msg)  assert_m(false, msg)

#if defined(DEVELOPER)
#define log_print(format, ...) _log_print(format, __VA_ARGS__)
#else
#define log_print(format, ...)
#endif

internal void _log_print(const char* format, ...) {
    static char print_buffer[4096];

    va_list args;
    va_start(args, format);
    _vsnprintf(print_buffer, 4096, format, args);
    va_end(args);

    platform_print_to_console(print_buffer);
}

internal u64 millis() {
    timespec t;

    timespec_get(&t, TIME_UTC);

    return (t.tv_sec * 1000) + (t.tv_nsec / 1000000);
}

internal u64 hash_string(const char *key) {
    u64 result = 1;

    const char *walker = key;

    while (walker && walker[0]) {
        result *= walker[0] * 33;

        ++walker;
    }

    return result;
}

internal u64 hash_string(const char *key, i32 length) {
    u64 result = 1;

    const char *walker = key;

    while (walker && walker[0] && length > 0) {
        result *= walker[0] * 33;

        ++walker;
        --length;
    }

    return result;
}

internal char* copy_string(const char *src, u64 length) {
    char *result = (char *)calloc((size_t)length + 1, 1);
    strncpy(result, src, (size_t)length);

    return result;
}

internal char* copy_string(const char *src) {
    u64 length = strlen(src);

    return copy_string(src, length);
}

#endif // UTILS_H
