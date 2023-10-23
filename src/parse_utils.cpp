internal bool is_end_of_line(char c) {
    return (c == '\n' || c == '\r');
}

internal bool is_alpha(char c) {
    return (
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z')
    );
}

internal bool is_numeric(char c) {
    return (c >= '0' && c <= '9');
}

internal bool is_space(char c) {
    return (
        c == ' ' ||
        c == '\t' ||
        c == '\v' ||
        c == '\f'
    );
}

// @Todo: this should be a platform code
internal bool read_whole_file(const char *name, Array<char> &buffer) {
    FILE *f;

    errno_t open_err = fopen_s(&f, name, "rb");

    if (open_err) {
        log_print("Error opening file: %s (err no.: %d)\n", name, open_err);
        return false;
    }

    fseek(f, 0, SEEK_END);
    u64 size = ftell(f);
    rewind(f);

    allocate_array(buffer, (size_t)size + 1);
    memset(buffer.data, 0, (size_t)size + 1);

    fread(buffer.data, sizeof(char), (size_t)size, f);

    buffer.length = (size_t)size + 1;

    fclose(f);

    return true;
}

// @Todo: this should be a platform code
internal bool write_to_file(const char *path, Array<char> &buffer) {
    FILE *f;

    errno_t open_err = fopen_s(&f, path, "w");

    if (open_err) {
        log_print("Error opening file: %s (err no.: %d)\n", path, open_err);
        return false;
    }

    fwrite(buffer.data, sizeof(char), buffer.length, f);

    fclose(f);

    return true;
}

// Eats only spaces and tabs
internal char* eat_spacing(char *buffer) {
    while (is_space(*buffer)) buffer++;

    return buffer;
}

internal char* eat_until_whitespace(char *buffer) {
    while (*buffer != '\0' && !is_space(*buffer) && !is_end_of_line(*buffer)) buffer++;

    return buffer;
}

internal char* eat_until(char *buffer, char until) {
    while (*buffer != '\0' && *buffer != until) buffer++;

    return buffer;
}
