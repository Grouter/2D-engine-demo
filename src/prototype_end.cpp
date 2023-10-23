internal void draw_prototype_end() {
    Font *font = get_font(FontResource_MEDIUM);

    f32 font_height_units = font->size * game_state->pixels_to_units;

    f32 screen_width = game_state->viewport.width * game_state->pixels_to_units;
    f32 screen_height = game_state->viewport.height * game_state->pixels_to_units;

    f32 draw_y = screen_height * 0.5f;

    {
        char *text = "Thank you for playing!";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 2.0f;

    {
        char *text = "This is all (for now)...";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }
}
