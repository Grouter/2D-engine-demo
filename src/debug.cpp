internal void draw_debug_info(f32 dt) {
    Font *font = get_font(FontResource_SMALL);

    f32 line_offset = font->size * game_state->pixels_to_units * 1.2f;

    Vector3 position = V3_ZERO;
    position.y = (f32)game_state->viewport.height * game_state->pixels_to_units;
    position.y -= FONT_SIZE_SMALL * game_state->pixels_to_units;
    position.z = ZLayer_UI;

    if (program_state == ProgramState_EDITOR) {
        f32 screen_width = game_state->viewport.width * game_state->pixels_to_units;
        position.x = screen_width * EDITOR_WIDTH_RATIO;
    }

    const u64 DEBUG_TEXT_BUFFER_SIZE = 64;
    static char debug_text_buffer[DEBUG_TEXT_BUFFER_SIZE];

    Color debug_color = make_color(150, 150, 150);

    // FPS
    {
        snprintf(debug_text_buffer, DEBUG_TEXT_BUFFER_SIZE, "FPS: %.2f", 1.0f / dt);

        draw_text(font, debug_text_buffer, position, debug_color);

        position.y -= line_offset;
    }

    // Entity count
    {
        snprintf(
            debug_text_buffer,
            DEBUG_TEXT_BUFFER_SIZE,
            "Rendering %d entities",
            game_state->entities.base_entities.stored
        );

        draw_text(font, debug_text_buffer, position, debug_color);

        position.y -= line_offset;
    }

    // Current level
    {
        snprintf(debug_text_buffer, DEBUG_TEXT_BUFFER_SIZE, "Current level: %s", game_state->current_level);

        draw_text(font, debug_text_buffer, position, debug_color);

        position.y -= line_offset;
    }

    // Current program state
    switch (program_state) {
        case ProgramState_EDITOR : {
            draw_text(font, "Program: EDITOR", position, debug_color);

            position.y -= line_offset;
        } break;

        case ProgramState_GAMEPLAY : {
            draw_text(font, "Program: GAMEPLAY", position, debug_color);

            position.y -= line_offset;
        } break;

        case ProgramState_PROTOTYPE_INTRO : {
            draw_text(font, "Program: INTRO", position, debug_color);

            position.y -= line_offset;
        } break;

        default : {
            draw_text(font, "Program: UNKNOWN", position, debug_color);

            position.y -= line_offset;
        }
    }

    {
        snprintf(debug_text_buffer, DEBUG_TEXT_BUFFER_SIZE, "Last overworld: %d", game_state->last_overworld_index);
        draw_text(font, debug_text_buffer, position, debug_color);
        position.y -= line_offset;
    }

    {
        bool is_in_overworld = GAME_LEVELS[game_state->current_level_index].type == LevelType::Overworld;

        snprintf(debug_text_buffer, DEBUG_TEXT_BUFFER_SIZE, "Overworld: %d", is_in_overworld);
        draw_text(font, debug_text_buffer, position, debug_color);
        position.y -= line_offset;
    }

    {
        snprintf(debug_text_buffer, DEBUG_TEXT_BUFFER_SIZE, "Last OW pos: %d, %d", game_state->overworld_last_position.x, game_state->overworld_last_position.y);
        draw_text(font, debug_text_buffer, position, debug_color);
        position.y -= line_offset;
    }

    if (game_state->current_level_index != -1) {
        snprintf(debug_text_buffer, DEBUG_TEXT_BUFFER_SIZE, "Solved: %d", game_state->levels_solved_state[game_state->current_level_index]);
        draw_text(font, debug_text_buffer, position, debug_color);
        position.y -= line_offset;
    }
}

internal void draw_debug_tile_states() {
    Font *font = get_font(FontResource_SMALL);

    Vector3 draw_pos;
    draw_pos.x = game_state->tile_area_offset.x;
    draw_pos.y = game_state->tile_area_offset.y + (font->size * game_state->pixels_to_units * 0.5f);
    draw_pos.z = ZLayer_UI;

    for (i32 row = 0; row < game_state->grid_rows; row++) {
        draw_pos.x = game_state->tile_area_offset.x;

        for (i32 col = 0; col < game_state->grid_cols; col++) {
            TileState *p = search_lookup_grid(col, row);

            if (p) {
                if (p->bottom_layer.type != EntityType::None) {
                    draw_text(font, "Block", draw_pos, Color_WHITE);
                }

                draw_pos.y += game_state->unit_tile_half_size;

                if (p->top_layer.type != EntityType::None) {
                    draw_text(font, "Block", draw_pos, Color_WHITE);
                }

                draw_pos.y -= game_state->unit_tile_half_size;
            }

            draw_pos.x += game_state->unit_tile_size;
        }

        draw_pos.y += game_state->unit_tile_size;
    }
}

internal void draw_debug_tile_edges() {
    Edge *it;
    array_foreach(game_state->edges, it) {
        Vector2 position = (it->a + it->b) * 0.5f;
        f32 edge_length = distance(it->a, it->b);

        Vector2 scale = make_vector2(0.2f);

        if (it->a.x == it->b.x) {   // Vert
            scale.y = edge_length;
        }
        else {  // Hoirz
            scale.x = edge_length;
        }

        draw_rect(position, scale, 0.0f, ZLayer_DEFAULT);
        draw_rect(it->a, make_vector2(0.2f), 0.0f, ZLayer_DEFAULT, Color_BLUE);
        draw_rect(it->b, make_vector2(0.2f), 0.0f, ZLayer_DEFAULT, Color_BLUE);
    }
}

#if defined(DEVELOPER)
#define draw_debug_window() _draw_debug_window()
#else
#define draw_debug_window()
#endif

internal void _draw_debug_window() {
    ImGuiIO io = ImGui::GetIO();

    ImGui::Begin("Debug");

    ImGui::Checkbox("Tile states", &debug_state->draw_tile_states);
    ImGui::Checkbox("Tile edges", &debug_state->draw_tile_edges);

    ImGui::End();
}
