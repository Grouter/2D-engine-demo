// @Todo Drawing the entity that is being built

internal void toggle_editor() {
    // Exiting editor
    if (program_state == ProgramState_EDITOR) {
        program_state = ProgramState_GAMEPLAY;

        load_level(game_state->current_level);
    }
    // Entering editor
    else if (program_state == ProgramState_GAMEPLAY) {
        program_state = ProgramState_EDITOR;

        load_level(game_state->current_level);

        editor_data.tile_selected = false;
    }
}

internal void editor_recalculate_texture_seams() {
    Vector2i directions_to_check[] = {
        V2i_UP,
        V2i_RIGHT,
        V2i_DOWN,
        V2i_LEFT,
    };

    Entity *entity;

    bucket_array_foreach(game_state->entities.base_entities, entity) {
        if (entity->type == EntityType::Wall) {
            u32 number_of_neighbor_walls = 0;
            bool neighbor_walls_occupied[] = { false, false, false, false };

            for (u32 i = 0; i < 4; i++) {
                Vector2i check_position = entity->grid_position + directions_to_check[i];

                TileState *neighbor = search_lookup_grid(check_position);

                if (neighbor && neighbor->top_layer.type == EntityType::Wall) {
                    ++number_of_neighbor_walls;
                    neighbor_walls_occupied[i] = true;
                }
            }

            if (number_of_neighbor_walls == 0) {
                entity->bitmap_atlas_position.x = 0;
                entity->bitmap_atlas_position.y = 4;
            }
            else if (number_of_neighbor_walls == 1) {
                entity->bitmap_atlas_position.y = 4;

                if (neighbor_walls_occupied[0]) entity->bitmap_atlas_position.x = 2;
                else if (neighbor_walls_occupied[1]) entity->bitmap_atlas_position.x = 3;
                else if (neighbor_walls_occupied[2]) entity->bitmap_atlas_position.x = 4;
                else if (neighbor_walls_occupied[3]) entity->bitmap_atlas_position.x = 5;
            }
            else if (number_of_neighbor_walls == 2) {
                entity->bitmap_atlas_position.y = 4;

                if (neighbor_walls_occupied[0] && neighbor_walls_occupied[1]) entity->bitmap_atlas_position.x = 6;
                else if (neighbor_walls_occupied[1] && neighbor_walls_occupied[2]) entity->bitmap_atlas_position.x = 7;
                else if (neighbor_walls_occupied[2] && neighbor_walls_occupied[3]) entity->bitmap_atlas_position.x = 8;
                else if (neighbor_walls_occupied[3] && neighbor_walls_occupied[0]) entity->bitmap_atlas_position.x = 9;
                else if (neighbor_walls_occupied[3] && neighbor_walls_occupied[0]) entity->bitmap_atlas_position.x = 9;
                else if (neighbor_walls_occupied[0] && neighbor_walls_occupied[2]) entity->bitmap_atlas_position.x = 10;
                else if (neighbor_walls_occupied[1] && neighbor_walls_occupied[3]) entity->bitmap_atlas_position.x = 11;
            }
            else if (number_of_neighbor_walls == 3) {
                entity->bitmap_atlas_position.y = 4;

                if (neighbor_walls_occupied[0] && neighbor_walls_occupied[1] && neighbor_walls_occupied[2]) entity->bitmap_atlas_position.x = 12;
                else if (neighbor_walls_occupied[1] && neighbor_walls_occupied[2] && neighbor_walls_occupied[3]) entity->bitmap_atlas_position.x = 13;
                else if (neighbor_walls_occupied[2] && neighbor_walls_occupied[3] && neighbor_walls_occupied[0]) entity->bitmap_atlas_position.x = 14;
                else if (neighbor_walls_occupied[3] && neighbor_walls_occupied[0] && neighbor_walls_occupied[1]) entity->bitmap_atlas_position.x = 15;
            }
            else {
                entity->bitmap_atlas_position.x = 1;
                entity->bitmap_atlas_position.y = 4;
            }
        }
    }}
}

internal void editor_on_remove_tile() {
    remove_flagged_entities(game_state->entities);
    update_lookup_grid();
}

internal void _draw_entity_editor(EntityPointer *e_ptr) {
    ImGui::Text("Type: %s", ENTITY_STRING_NAMES[(u32)e_ptr->type]);

    if (e_ptr->type == EntityType::None) return;

    Entity *e = &game_state->entities.base_entities[e_ptr->id.location];

    ImGui::Text("Flags: %d", e->flags);
    ImGui::Text("Z Layer: %f", e->z_layer);
    ImGui::Text("Scale: [%f, %f]", e->scale.x, e->scale.y);

    {
        bool visible_only_in_editor = e->flags.is_visible_only_in_editor;
        if (ImGui::Checkbox("Only Editor Visible", &visible_only_in_editor)) {
            e->flags.is_visible_only_in_editor = visible_only_in_editor;
        }
    }

    if (e->flags.is_powered) {
        if (ImGui::InputScalar("Pwr Grp", ImGuiDataType_U32, &e->power_group)) {
            e->power_group = CLAMP(e->power_group, 0, POWER_GROUP_COUNT - 1);
        }
    }

    if (ImGui::InputScalar("Rotation", ImGuiDataType_U32, &e->logical_rotation)) {
        e->logical_rotation = CLAMP(e->logical_rotation, 0, 3);
    }

    switch (e_ptr->type) {
        case EntityType::LevelTransition : {
            EntityLevelTransition *transition = &game_state->entities.entity_data[e->data_location].level_transition;

            {
                bool to_puzzle = (bool)transition->is_to_puzzle;
                ImGui::Checkbox("To Puzzle", &to_puzzle);
                transition->is_to_puzzle = to_puzzle;
            }

            ImGui::InputInt("Tran Link", &transition->transition_link);

            {
                if (transition->next_level_index == -1) {
                    ImGui::Text("Transition to: NONE");
                }
                else {
                    const char *name = GAME_LEVELS[transition->next_level_index].name;
                    ImGui::Text("Transition to: %s", name);

                    if (editor_data.transition_level_buffer[0] == '\0') {
                        strncpy(editor_data.transition_level_buffer, name, MAX_LEVEL_NAME_LEN);
                    }
                }

                ImGui::InputText("Tranition", editor_data.transition_level_buffer, MAX_LEVEL_NAME_LEN - 1, ImGuiInputTextFlags_CharsNoBlank);

                if (ImGui::Button("Set Transition")) {
                    u64 hash = hash_string(editor_data.transition_level_buffer);

                    u32 level_index = 0;
                    const char *level_name = get_level_name(hash, &level_index);

                    if (level_name != nullptr) {
                        transition->next_level_index = level_index;
                    }
                }

                if (ImGui::Button("Clear Transition")) {
                    transition->next_level_index = -1;
                }
            }
        } break;
    }
}

internal void draw_editor(f32 dt) {
    f32 screen_width = game_state->viewport.width * game_state->pixels_to_units;
    f32 screen_height = game_state->viewport.height * game_state->pixels_to_units;

    ImGuiIO io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * EDITOR_WIDTH_RATIO, io.DisplaySize.y));
    ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize);

    ImVec2 editor_window_size = ImGui::GetWindowSize();

    update_lookup_grid();

    // Draw tile plane
    {
        Vector2 position = make_vector2(screen_width * 0.5f, screen_height * 0.5f);

        draw_rect(position, game_state->tile_area_size, 0.0f, -0.9f, make_color(20));
    }

    // Save button
    if (ImGui::Button("Save", ImVec2(editor_window_size.x, 50.0f))) {
        editor_recalculate_texture_seams();

        bool success = save_level(game_state->current_level, game_state);

        if (success) log_print("Level saved\n");
        else log_print("Error while saving level\n");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Deselect
    if (editor_data.tile_selected && ImGui::Button("Deselect", ImVec2(editor_window_size.x, 50.0f))) {
        editor_data.tile_selected = false;
        editor_data.selected_tile = nullptr;
    }

    if (editor_data.tile_selected) {
        TileState* selected_tile = editor_data.selected_tile;

        ImGui::Text("Edges: %d %d %d %d", selected_tile->has_edge[0], selected_tile->has_edge[1], selected_tile->has_edge[2], selected_tile->has_edge[3]);
        ImGui::Text("Logical Position: [%d, %d]", editor_data.tile_selection_position.x, editor_data.tile_selection_position.y);

        ImGui::Spacing();
        ImGui::Spacing();

        // Top tile layer
        {
            ImGui::Text("Top Layer");
            ImGui::Indent();

            _draw_entity_editor(&selected_tile->top_layer);

            ImGui::Spacing();

            if (selected_tile->top_layer.type != EntityType::None && ImGui::Button("Destroy Top Layer!", ImVec2(editor_window_size.x, 30.0f))) {
                flag_remove_tile(&game_state->entities, selected_tile, EntityLogicLayer::Top);
                editor_on_remove_tile();
            }

            ImGui::Unindent();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Bot tile layer
        {
            ImGui::Text("Bottom Layer");
            ImGui::Indent();

            _draw_entity_editor(&selected_tile->bottom_layer);

            ImGui::Spacing();

            if (selected_tile->bottom_layer.type != EntityType::None && ImGui::Button("Destroy Bottom Layer!", ImVec2(editor_window_size.x, 30.0f))) {
                flag_remove_tile(&game_state->entities, selected_tile, EntityLogicLayer::Bottom);
                editor_on_remove_tile();
            }

            ImGui::Unindent();
        }

        // Draw tile selection
        {
            Vector2 position = grid_pos_to_real_pos(editor_data.tile_selection_position.x, editor_data.tile_selection_position.y);

            f32 effect = (sin(game_state->time_elapsed * 5.0f) + 1) * 0.5f;
            Color selection_color = make_color(0, 255, 0, (u8)(effect * 100));

            draw_rect(position, make_vector2(game_state->unit_tile_size), 0.0f, ZLayer_UI, selection_color);
        }
    }
    else {
        ImGui::Combo("Select", &editor_data.selected_entity_spawn, ENTITY_STRING_NAMES, STATIC_ARRAY_CAPACITY(ENTITY_STRING_NAMES));

        if (editor_data.is_building) {
            if (ImGui::Button("Stop building", ImVec2(editor_window_size.x, 30.0f))) {
                editor_data.is_building = false;
            }
        }
        else if (editor_data.selected_entity_spawn != 0) {  // Cannot spawn None entity
            if (ImGui::Button("Spawn!", ImVec2(editor_window_size.x, 30.0f))) {
                editor_data.is_building = true;
            }
        }

        editor_data.transition_level_buffer[0] = '\0';
    }

    ImGui::End();
}

internal void editor_handle_key(KeyInput *key) {
    if (!editor_data.is_building && editor_data.tile_selected) {
        TileState* selected_tile = editor_data.selected_tile;

        Vector2i move_delta = {};

        switch (key->virtual_code) {
            case VK_UP : {
                move_delta.y = 1;
            } break;

            case VK_LEFT : {
                move_delta.x = -1;
            } break;

            case VK_DOWN : {
                move_delta.y = -1;
            } break;

            case VK_RIGHT : {
                move_delta.x = 1;
            } break;
        }

        bool no_input = move_delta.x == 0 && move_delta.y == 0;

        if (!no_input) {
            bool move_top_layer = true;

            if (key->ctrl_down) {
                move_top_layer = false;
            }

            Entity *to_move = nullptr;

            if (move_top_layer && selected_tile->top_layer.type != EntityType::None) {
                to_move = &game_state->entities.base_entities[selected_tile->top_layer.id.location];
            }
            else if (!move_top_layer && selected_tile->bottom_layer.type != EntityType::None) {
                to_move = &game_state->entities.base_entities[selected_tile->bottom_layer.id.location];
            }

            if (to_move) {
                Vector2i new_position = to_move->grid_position + move_delta;

                TileState* target_tile = search_lookup_grid(new_position);

                if (target_tile) {
                    bool can_move = (
                        (move_top_layer && target_tile->top_layer.type == EntityType::None)
                        ||
                        (!move_top_layer && target_tile->bottom_layer.type == EntityType::None)
                    );

                    if (can_move) {
                        sync_set_grid_position(*to_move, new_position.x, new_position.y);
                        editor_data.selected_tile = target_tile;
                        editor_data.tile_selection_position = new_position;
                    }
                }
            }
        }
    }
}

internal void editor_handle_char_input(KeyInput *key) {}

internal void editor_handle_left_mouse_button() {
    bool is_mouse_over_valid_tile = is_valid_grid_position(input_state->mouse_grid.x, input_state->mouse_grid.y);

    if (editor_data.is_building) {
        if (is_mouse_over_valid_tile) {
            TileState *target_tile = search_lookup_grid(input_state->mouse_grid);

            EntityType to_build_type = (EntityType)editor_data.selected_entity_spawn;
            Entity *created_entity = create_entity_from_type(&game_state->entities, to_build_type);

            if (target_tile != nullptr && entity_can_be_placed(created_entity, target_tile)) {
                sync_set_grid_position(*created_entity, input_state->mouse_grid.x, input_state->mouse_grid.y);

                editor_recalculate_texture_seams();

                log_print("Building: %s\n", ENTITY_STRING_NAMES[editor_data.selected_entity_spawn]);
            }
            else {
                created_entity->flags.destroy = true;
            }
        }

        editor_data.transition_level_buffer[0] = '\0';

        return;
    }

    if (is_mouse_over_valid_tile) {
        editor_data.tile_selection_position = input_state->mouse_grid;

        editor_data.tile_selected = true;
        editor_data.selected_tile = search_lookup_grid(editor_data.tile_selection_position);

        editor_data.transition_level_buffer[0] = '\0';
    }
    else {
        editor_data.tile_selected = false;
    }
}

internal void editor_handle_right_mouse_button() {
    if (is_valid_grid_position(input_state->mouse_grid.x, input_state->mouse_grid.y)) {
        flag_remove_tile(&game_state->entities, input_state->mouse_grid);
        editor_on_remove_tile();
    }
}
