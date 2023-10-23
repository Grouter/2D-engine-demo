internal void draw_text_input(TextInput &input, f32 dt, Vector3 bottom_left, Vector2 size, Font *font, f32 font_scale) {
    f32 font_unit_height = font->size * game_state->pixels_to_units * font_scale;

    // Draw text
    {
        Vector3 position = make_vector3(
            bottom_left.x,
            bottom_left.y + (size.y * 0.5f) - (font_unit_height * 0.5f),
            bottom_left.z
        );

        draw_text(font, input.contents.data, position, input.text_color, font_scale);
    }

    // Draw cursor
    if ((input.last_input_time < TEXT_INPUT_BLINK_DELAY) || (fmod(input.last_input_time, 1.0f) <= 0.5f)) {
        f32 text_width = get_string_width_in_units(font, input.contents.data, font_scale);
        f32 cursor_width = 0.1f;
        f32 cursor_offset = 0.02f;

        Vector2 cursor_size = make_vector2(
            cursor_width,
            font_unit_height
        );

        Vector2 position = make_vector2(
            bottom_left.x + text_width + cursor_width * 0.5f + cursor_offset,
            bottom_left.y + (size.y * 0.5f)
        );

        draw_rect(position, cursor_size, 0.0f, bottom_left.z);
    }

    input.last_input_time += dt;
}

internal void draw_text_input_c(TextInput &input, Font *font, Vector2 position, f32 layer, Vector2 size, f32 dt, f32 font_scale) {
    f32 font_unit_height = font->size * game_state->pixels_to_units * font_scale;

    // Draw text
    {
        Vector3 text_position = make_vector3(
            position.x - size.x * 0.5f,
            position.y - (size.y * 0.5f) + (font_unit_height * 0.5f),
            layer + 0.01f
        );

        draw_text(font, input.contents.data, text_position, input.text_color, font_scale);
    }

    // Draw cursor
    if (input.focused && ((input.last_input_time < TEXT_INPUT_BLINK_DELAY) || (fmod(input.last_input_time, 1.0f) <= 0.5f))) {
        f32 text_width = get_string_width_in_units(font, input.contents.data, font_scale);
        f32 cursor_width = 0.1f;
        f32 cursor_offset = 0.02f;

        Vector2 cursor_size = make_vector2(
            cursor_width,
            font_unit_height
        );

        Vector2 cursor_position = make_vector2(
            position.x - (size.x * 0.5f) + text_width + cursor_width * 0.5f + cursor_offset,
            position.y
        );

        draw_rect(cursor_position, cursor_size, 0.0f, layer, Color_BLUE);
    }

    // Draw BG
    {
        Vector2 bg_position = make_vector2(position.x, position.y);
        draw_rect(bg_position, size, 0.0f, layer);
    }

    input.last_input_time += dt;
}

internal void text_input_handle_char(TextInput &input, KeyInput &key) {
    if (key.character == VK_BACK) {
        input.delete_last_char();
    }
    else if (key.character >= 32 && key.character <= 126) {
        input.add_char((char)key.character);
    }
}
