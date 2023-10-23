internal void draw_prototype_intro() {
    Font *font = get_font(FontResource_MEDIUM);

    f32 font_height_units = font->size * game_state->pixels_to_units;

    f32 screen_width = game_state->viewport.width * game_state->pixels_to_units;
    f32 screen_height = game_state->viewport.height * game_state->pixels_to_units;

    f32 draw_y = screen_height - font_height_units * 5.0f;

    {
        char *text = "This is a prototype version of the game! Please do not distribute.";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 2.0f;

    {
        char *text = "Visuals are subject to change.";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 2.0f;

    {
        char *text = "The prototype contains 15 puzzle levels.";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 4.0f;

    {
        char *text = "Move with: WASD or Arrow keys";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 2.0f;

    {
        char *text = "Restart level with: R";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 2.0f;

    {
        char *text = "Quit puzzle to level selection with: Q";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 2.0f;

    {
        char *text = "Quit the program with: ESC";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }

    draw_y -= font_height_units * 5.0f;

    {
        char *text = "Press SPACE to play!";
        f32 text_size = get_string_width_in_units(font, text, 1.0f);

        Vector3 position = {};
        position.x = (screen_width * 0.5f) - text_size * 0.5f;
        position.y = draw_y;

        draw_text(font, text, position, Color_WHITE);
    }
}
