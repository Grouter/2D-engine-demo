// This file is made super dumb and contains only loading/saving for development version of the game.
// In this shipping version, we will certainly not use strings and string comaprison! We will read this data
// from a binary asset file which will directly map to a structure data layout.

internal bool new_level(const char *name) {
    char path_buffer[64];
    snprintf(path_buffer, 64, "levels/%s.level", name);

    // Check for existence
    {
        std::ifstream potential_new_file(path_buffer);

        // If exists, fail!
        if (potential_new_file.good()) return false;

        potential_new_file.close();
    }

    std::ofstream new_level_file(path_buffer);

    if (!new_level_file.good()) return false;

    new_level_file << "level {\n";
    new_level_file << "    required_goals: 1\n";
    new_level_file << "}\n";

    new_level_file.close();

    return true;
}

// @Todo pass index instead of a string
internal bool load_level(const char* name, i32 transition_link = -1) {
    log_print("Loading level %s\n", name);

    i32 level_to_load_index = -1;
    Level level_to_load = get_level_data(name, &level_to_load_index);

#if !defined(DEVELOPER)
    if (level_to_load.type == LevelType::None || level_to_load_index == -1) {
        log_print("Level with that name is not registered!\n");
        return false;
    }
#endif

    Level old_level;

    if (level_to_load_index == -1) {
        old_level.type = LevelType::None;
    }
    else {
        old_level = GAME_LEVELS[game_state->current_level_index];
    }

    game_state->current_level_index = level_to_load_index;

    char path_buffer[MAX_LEVEL_NAME_LEN + 13];
    snprintf(path_buffer, STATIC_ARRAY_CAPACITY(path_buffer), "levels/%s.level", name);

    Array<char> level_file;

    {
        bool success = read_whole_file(path_buffer, level_file);
        if (!success) {
            log_print("Could not load level %s\n", path_buffer);
            return false;
        }
    }

    // Clear game
    {
        // @Todo: We can just reallocate and zero-out this memory from our arena.

        game_state->entities.base_entities.clear();
        game_state->entities.entity_data.clear();

        game_state->move_transactions.clear();
        game_state->player_move_commands.clear();

        memset(game_state->power_states, 0, STATIC_ARRAY_CAPACITY(game_state->power_states));

        game_state->game_tick_timer = 0.0f;
        game_state->ticks_elapsed = 0;
        game_state->belt_animation = 0.0f;

        const i32 DEFAULT_GRID_ROWS = 16;
        const i32 DEFAULT_GRID_COLS = 16;

        recalculate_tile_grid(DEFAULT_GRID_COLS, DEFAULT_GRID_ROWS);
    }

    // Transition link, that matches given transition link in this level
    Entity *target_transition_link = nullptr;

    u32 player_count = 0;

    {
        Tokenizer tokenizer = tokenize(level_file.data, level_file.length);

        while (1) {
            Token block_type = tokenizer_get_real_token(&tokenizer);

            if (block_type.type == TokenVariant::Identifier) {
                tokenizer_require_token(&tokenizer, TokenVariant::OpenBrace);
                if (tokenizer.error)
                    continue;

                if (token_matches(block_type, "level")) {
                    while (1) {
                        Token field_name = tokenizer_get_real_token(&tokenizer);

                        if (field_name.type == TokenVariant::CloseBrace) break;

                        if (field_name.type == TokenVariant::Identifier) {
                            tokenizer_require_token(&tokenizer, TokenVariant::Colon);
                            if (tokenizer.error) continue;

                            if (token_matches(field_name, "grid")) {
                                Vector2i grid = make_vector2i(game_state->grid_cols, game_state->grid_rows);

                                parse_i32_2(&tokenizer, &grid.x, &grid.y);

                                game_state->grid_cols = grid.x;
                                game_state->grid_rows = grid.y;

                                recalculate_tile_grid(game_state->grid_cols, game_state->grid_rows);
                            }
                            else {
                                log_print(
                                    "Error: unsupported field '%.*s' on line %d\n",
                                    field_name.length,
                                    field_name.text,
                                    tokenizer.line_number
                                );
                            }
                        }
                    }
                }
                else {
                    EntityType e_type = entity_type_from_string(block_type.text, block_type.length);

                    if (e_type == EntityType::None || e_type == EntityType::Count) {
                        log_print("Error: unknown block in level file: %.*s\n", block_type.length, block_type.text);
                    }
                    else {
                        Entity *entity = create_entity_from_type(&game_state->entities, e_type);

                        if (e_type == EntityType::Player) {
                            ++player_count;
                        }

                        // Parse the base entity
                        while (1) {
                            Token field_name = tokenizer_get_real_token(&tokenizer);

                            if (field_name.type == TokenVariant::CloseBrace) break;

                            if (field_name.type == TokenVariant::Identifier) {
                                tokenizer_require_token(&tokenizer, TokenVariant::Colon);
                                if (tokenizer.error) continue;

                                if (token_matches(field_name, "flags")) {
                                    u32 flags = 0;

                                    bool success = parse_u32_1(&tokenizer, &flags);
                                    if (!success) continue;

                                    entity->flags.raw = flags;
                                }
                                else if (token_matches(field_name, "power_group")) {
                                    bool success = parse_u32_1(&tokenizer, &entity->power_group);

                                    if (!success) continue;
                                }
                                else if (token_matches(field_name, "z_layer")) {
                                    entity->z_layer = 0.0f;

                                    bool success = parse_f32_1(&tokenizer, &entity->z_layer);
                                    if (!success) continue;
                                }
                                else if (token_matches(field_name, "logic_level")) {
                                    i32 logic_layer = 0;

                                    bool success = parse_i32_1(&tokenizer, &logic_layer);
                                    if (!success) continue;

                                    entity->logic_level = (EntityLogicLayer)logic_layer;
                                }
                                else if (token_matches(field_name, "position")) {
                                    Vector2i position = {};

                                    bool success = parse_i32_2(&tokenizer, &position.x, &position.y);
                                    if (!success) continue;

                                    sync_set_grid_position(*entity, position.x, position.y);
                                }
                                else if (token_matches(field_name, "rotation")) {
                                    i32 rotation = 0;

                                    bool success = parse_i32_1(&tokenizer, &rotation);
                                    if (!success) continue;

                                    entity->logical_rotation = (u32)rotation;
                                }
                                else if (token_matches(field_name, "scale")) {
                                    entity->scale = V2_ONE;

                                    bool success = parse_f32_2(&tokenizer, &entity->scale.x, &entity->scale.y);
                                    if (!success) continue;
                                }
                                else if (token_matches(field_name, "bitmap_atlas_position")) {
                                    Vector2i position = {};

                                    bool success = parse_i32_2(&tokenizer, &position.x, &position.y);
                                    if (!success) continue;

                                    entity->bitmap_atlas_position = position;
                                }
                                // We handle special cases for entity types
                                else {
                                    switch (entity->type) {
                                        case EntityType::LevelTransition : {
                                            EntityLevelTransition *transition = &game_state->entities.entity_data[entity->data_location].level_transition;

                                            if (token_matches(field_name, "next_level")) {
                                                Token level_token = tokenizer_get_real_token(&tokenizer);

                                                if (level_token.type == TokenVariant::String) {
                                                    transition->next_level_index = get_level_index(level_token.text, (i32)level_token.length);
                                                }
                                                else if (level_token.type == TokenVariant::Number) {
                                                    transition->next_level_index = level_token.parsed_number_i32;
                                                }
                                                else {
                                                    continue;
                                                }
                                            }
                                            else if (token_matches(field_name, "transition_link")) {
                                                bool success = parse_i32_1(&tokenizer, &transition->transition_link);
                                                if (!success) continue;

                                                if (transition->transition_link == transition_link) {
                                                    target_transition_link = entity;
                                                }
                                            }
                                            else if (token_matches(field_name, "to_puzzle")) {
                                                i32 to_puzzle = 0;

                                                bool success = parse_i32_1(&tokenizer, &to_puzzle);
                                                if (!success) continue;

                                                transition->is_to_puzzle = (bool16)to_puzzle;
                                            }
                                        } break;

                                        default : {
                                            log_print(
                                                "Error: unsupported field '%.*s' on line %d\n",
                                                field_name.length,
                                                field_name.text,
                                                tokenizer.line_number
                                            );
                                        };
                                    }
                                }
                            }
                        }

                        if (program_state != ProgramState_EDITOR && e_type == EntityType::LevelStart) {
                            Entity *player = create_player(&game_state->entities);
                            player->logical_rotation = entity->logical_rotation;
                            sync_set_grid_position(*player, entity->grid_position.x, entity->grid_position.y);

                            ++player_count;
                        }
                    }
                }
            }
            else if (block_type.type == TokenVariant::EndOfStream) {
                break;
            }
            else {
                log_print("Error: unknown token in %s on line %d\n", path_buffer, tokenizer.line_number);
            }
        }
    }

    // Lookup grid check
    {
        assert((game_state->grid_cols * game_state->grid_rows) < (MAX_GRID_ROWS * MAX_GRID_COLS));
    }

    update_lookup_grid();

    // Transition link stuff
    if (transition_link >= 0) {
        if (target_transition_link == nullptr) {
            log_print("Level %s has no transition tile with link %d!\n", name, transition_link);
        }
        else {
            Entity *player = create_player(&game_state->entities);
            player->logical_rotation = game_state->last_player_rotation;
            sync_set_grid_position(*player, target_transition_link->grid_position.x, target_transition_link->grid_position.y);

            ++player_count;
        }
    }

#if defined(DEVELOPER)
    // Save current level name
    strncpy(game_state->current_level, name, MAX_LEVEL_NAME_LEN);
#endif

    // Going back to the overworld
    if (level_to_load.type == LevelType::Overworld) {
        game_state->player_blink_effect = PLAYER_BLINK;
        game_state->last_overworld_index = level_to_load_index;

        if (old_level.type == LevelType::Puzzle && game_state->overworld_last_position.x != -1 && game_state->overworld_last_position.y != -1) {
            Entity *player = create_player(&game_state->entities);
            player->logical_rotation = game_state->last_player_rotation;
            sync_set_grid_position(*player, game_state->overworld_last_position.x, game_state->overworld_last_position.y);

            ++player_count;
        }
        else {
            game_state->player_blink_effect = 0.0f;
        }

        if (!game_state->levels_solved_state[game_state->current_level_index]) {
            bool all_puzzles_are_solved = true;

            Entity *entity;
            bucket_array_foreach(game_state->entities.base_entities, entity) {
                if (!all_puzzles_are_solved) break;

                if (entity->type == EntityType::LevelTransition) {
                    EntityLevelTransition *transition = &game_state->entities.entity_data[entity->data_location].level_transition;

                    if (transition->is_to_puzzle && !game_state->levels_solved_state[transition->next_level_index]) {
                        all_puzzles_are_solved = false;
                    }
                }
            }}

            game_state->levels_solved_state[game_state->current_level_index] = all_puzzles_are_solved;
        }

        // @Hardocde (remove walls in overworld)
        if (program_state == ProgramState_GAMEPLAY && game_state->levels_solved_state[game_state->current_level_index]) {
            Entity *entity;
            bucket_array_foreach(game_state->entities.base_entities, entity) {
                if (entity->logic_level == EntityLogicLayer::Top) {
                    if (entity->grid_position.x == 13) {
                        if (entity->grid_position.y == 8 || entity->grid_position.y == 7) {
                            entity->flags.destroy = true;
                        }
                    }
                }
            }}
        }
    }

    // Spawn a player for debug
    if (program_state != ProgramState_EDITOR && player_count == 0) {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            if (player_count > 0) continue;

            if (entity->type == EntityType::LevelTransition) {
                Entity *player = create_player(&game_state->entities);
                player->logical_rotation = game_state->last_player_rotation;
                sync_set_grid_position(*player, entity->grid_position.x, entity->grid_position.y);

                ++player_count;
            }
        }}
    }

    remove_flagged_entities(game_state->entities);
    update_lookup_grid();

    if (game_state->do_fov_fog) compute_tiles_edges(game_state->edges);

    log_print("Level %s loaded!\n", game_state->current_level);

    return true;
}

// @Todo Sort the entities before saving, so they are always serialized in the most efficient way
internal bool save_level(const char *file_name, GameState *game) {
    char path_buffer[MAX_LEVEL_NAME_LEN + 13];
    snprintf(path_buffer, STATIC_ARRAY_CAPACITY(path_buffer), "levels/%s.level", file_name);

    std::ofstream save_file(path_buffer);

    if (!save_file.is_open()) {
        log_print("Unable to open current save file!\n");
        return false;
    }

    // Level block
    {
        save_file << "level {\n";

        save_file << "  grid: " << game->grid_cols << ", " << game->grid_rows << "\n";

        save_file << "}\n";
    }

    save_file << '\n';

    Entity *e;
    bucket_array_foreach(game_state->entities.base_entities, e) {
        save_file << ENTITY_STRING_NAMES[(u32)e->type] << " {\n";

        save_file << "  flags: " << e->flags.raw << "\n";
        save_file << "  position: " << e->grid_position.x << ", " <<  e->grid_position.y << "\n";
        save_file << "  rotation: " << e->logical_rotation << "\n";

        if (e->flags.is_powered) {
            save_file << "  power_group: " << e->power_group << "\n";
        }

        switch (e->type) {
            case EntityType::LevelTransition : {
                EntityLevelTransition *transition = &game_state->entities.entity_data[e->data_location].level_transition;

                if (transition->next_level_index == -1) {
                    save_file << "  next_level: -1\n";
                }
                else {
                    const char *level = GAME_LEVELS[transition->next_level_index].name;
                    save_file << "  next_level: \"" << level << "\"\n";
                }

                save_file << "  transition_link: " << transition->transition_link << "\n";
                save_file << "  to_puzzle: " << transition->is_to_puzzle << "\n";
            } break;

            case EntityType::Wall : {
                save_file << "  bitmap_atlas_position: " << e->bitmap_atlas_position.x << ", " << e->bitmap_atlas_position.y << "\n";
            } break;
        }

        save_file << "}\n\n";
    }}

    save_file.close();

    return true;
}
