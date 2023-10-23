#if !defined(CONSOLE_H)
#define CONSOLE_H

#define CONSOLE_OPEN_SPEED 2.0f

struct Console {
    TextInput input;
    bool32 open     = false;
    f32 open_target = 0.0f;
    f32 openness    = 0.0f;
    StaticArray<char *> history;
};

internal void console_add_to_history(const char* format, ...);

#endif
