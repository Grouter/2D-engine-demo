internal void init_console() {
    Console *console = &game_state->console;
    *console = {};

    init_text_input(console->input, 64);
    console->input.text_color = make_color(222);

    allocate_array_from_block(console->history, 32, &game_state->permanent_memory);
}

internal void console_add_to_history(const char* format, ...) {
    static char buffer[64];

    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, 64, format, args);
    va_end(args);

    char* new_entry = copy_string(buffer);
    game_state->console.history.add(new_entry);
}

internal void clear_console() {
    Console *console = &game_state->console;

    console->history.clear();
}

internal void toggle_console() {
    Console *console = &game_state->console;

    console->open = !console->open;

    if (console->open) console->open_target = 0.5f;
    else console->open_target = 0.0f;
}

internal void draw_console(f32 dt) {
    Console *console = &game_state->console;

    if (!console->open) {
        if (console->openness != console->open_target) {
            console->openness = max(0.0f, console->openness - (CONSOLE_OPEN_SPEED * dt));
        }
        else return;
    }
    else {
        if (console->openness != console->open_target)
            console->openness = min(console->open_target, console->openness + (CONSOLE_OPEN_SPEED * dt));
    }

    Font *font = get_font(FontResource_MEDIUM);

    f32 screen_width = game_state->viewport.width * game_state->pixels_to_units;
    f32 screen_height = game_state->viewport.height * game_state->pixels_to_units;

    f32 text_scale = 0.6f;

    f32 input_height = font->size * text_scale * game_state->pixels_to_units * 1.5f;
    f32 history_line_height = font->size * text_scale * game_state->pixels_to_units * 1.5f;

    f32 text_left_padding = 0.05f;

    Vector2 background_size = make_vector2(screen_width, screen_height * console->openness);

    // Draw BG
    {
        Vector2 position = make_vector2(
            screen_width * 0.5f,
            screen_height - (background_size.y * 0.5f)
        );

        draw_rect(position, background_size, 0.0f, ZLayer_CONSOLE - 0.02f, make_color(10, 90, 30));
    }

    // Draw input rect
    {
        Vector2 position = make_vector2(
            screen_width * 0.5f,
            screen_height - background_size.y + input_height * 0.5f
        );

        Vector2 size = make_vector2(
            screen_width,
            input_height * 1.2f
        );

        draw_rect(position, size, 0.0f, ZLayer_CONSOLE - 0.01f, make_color(70, 10, 10));
    }

    // Draw Input
    {
        Vector3 position = make_vector3(
            text_left_padding,
            screen_height - background_size.y,
            ZLayer_CONSOLE
        );

        Vector2 size = make_vector2(
            screen_width,
            input_height
        );

        draw_text_input(console->input, dt, position, size, font, text_scale);
    }

    // Draw history
    {
        Color text_color = make_color(222);

        Vector3 position = make_vector3(
            text_left_padding,
            screen_height - background_size.y + input_height * 1.3f,
            ZLayer_CONSOLE
        );

        for (i32 i = (i32)console->history.length - 1; i >= 0; i--) {
            draw_text(font, console->history[i], position, text_color, text_scale);

            position.y += history_line_height;

            if (position.y > screen_height) break;
        }
    }
}

internal void console_handle_char_input(KeyInput *key) {
    Console *console = &game_state->console;

    if (key->character >= 32 && key->character <= 126) {
        console->input.add_char((char)key->character);
    }
    else if (key->character == VK_BACK) {
        console->input.delete_last_char();
    }
    else if (key->character == VK_RETURN) {
        if (console->input.contents.length == 0) return;

        console_add_to_history("> %s", console->input.contents.data);

        execute_command(&game_state->commands, console->input.contents.data);

        console->input.clear();
    }
}
