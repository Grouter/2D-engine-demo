#if !defined(STRING_BUILDER_H)
#define STRING_BUILDER_H

#if 0

// Will not auto zero terminate. User is responsible to provide such string and length!!
internal void append_str(Array<char> &buffer, char *src, u32 length) {
    // Reserve enough to copy
    {
        u64 free_space = buffer.capacity - buffer.length;
        i32 to_reserve = (i32)(length + 1) - (i32)free_space;    // +1 for '\0'

        if (to_reserve > 0) buffer.reserve(to_reserve);
    }

    memcpy(buffer.data, src, length);
    buffer.length += length;
}

internal void append_str(Array<char> &buffer, char *src) {
    u32 length = (u32)strlen(src);

    append_str(buffer, src, length);
}

internal void append_str_format(Array<char> &buffer, const char *format, ...) {
    static char output_buffer[2048];

    va_list args;
    va_start(args, format);
    _vsnprintf(output_buffer, 2048, format, args);
    va_end(args);

    append_str(buffer, output_buffer);
}

#endif

#endif
