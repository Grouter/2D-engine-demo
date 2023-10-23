internal void init_game() {
    game_state->camera = create_camera(VIRTUAL_WINDOW_W, VIRTUAL_WINDOW_H);

    // @Todo: can be done at compile time
    // Compute level hashes
    {
        u32 level_count = STATIC_ARRAY_CAPACITY(GAME_LEVELS);

        for (u32 i = 0; i < level_count; i++) {
            assert(strlen(GAME_LEVELS[i].name) <= MAX_LEVEL_NAME_LEN);

            _GAME_LEVEL_HASHES[i] = hash_string(GAME_LEVELS[i].name);

            assert(_GAME_LEVEL_HASHES[i] != 0);
        }
    }

    load_resources(*resources, &game_state->permanent_memory);

    init_renderer(&game_state->permanent_memory);

    // Stuff for edge detection and "polygonization"
    {
        allocate_array_from_block(game_state->edges, 512, &game_state->permanent_memory);
        allocate_array_from_block(game_state->fov_mesh, 512, &game_state->permanent_memory);

        glGenVertexArrays(1, &game_state->fov_vao);
        glBindVertexArray(game_state->fov_vao);

        glGenBuffers(1, &game_state->fov_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, game_state->fov_vbo);
        glBufferData(GL_ARRAY_BUFFER, game_state->fov_mesh.capacity * sizeof(game_state->fov_mesh[0]), game_state->fov_mesh.data, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    allocate_entity_storage(game_state->entities, &game_state->permanent_memory);

    allocate_array_from_block(game_state->move_transactions, 32, &game_state->permanent_memory);
    allocate_array_from_block(game_state->player_move_commands, 8, &game_state->permanent_memory);

#if defined(DEVELOPER)
    allocate_array_from_block(game_state->commands, 32, &game_state->permanent_memory);
    init_commands(&game_state->commands);

    init_console();
#endif

    // First level
    load_game_state(game_state);

    if (game_state->last_overworld_index != -1) {
        const char* last_overworld_name = GAME_LEVELS[game_state->last_overworld_index].name;

        load_level(last_overworld_name);
    }
    else {
        load_level("intro1");
    }

    update_lookup_grid();

    game_state->do_fov_fog = false;

    log_print("Game initialized! Permanent memory remaining: %u bytes.\n", (game_state->permanent_memory.bytes - game_state->permanent_memory.used));
}

internal void cleanup_game() {
    log_print("TODO: game cleanup here...\n");
}

internal void on_window_created(i32 pixel_width, i32 pixel_height) {
    game_state->window_width_pixels  = pixel_width;
    game_state->window_height_pixels = pixel_height;
}

internal void on_window_resize(u32 pixel_width, u32 pixel_height) {
    if (pixel_width == 0 || pixel_height == 0) return;

    game_state->window_width_pixels  = pixel_width;
    game_state->window_height_pixels = pixel_height;

    // Recalculate viewport to keep the aspect ratio
    {
        u32 w_ratio = pixel_width / TARGET_ASPECT_W;
        u32 h_ratio = pixel_height / TARGET_ASPECT_H;

        u32 target_ratio = min(w_ratio, h_ratio);
        if (target_ratio % 2 == 1) target_ratio -= 1;

        pixel_width = target_ratio * TARGET_ASPECT_W;
        pixel_height = target_ratio * TARGET_ASPECT_H;

        game_state->viewport.left   = (game_state->window_width_pixels / 2) - (pixel_width / 2);
        game_state->viewport.bottom = (game_state->window_height_pixels / 2) - (pixel_height / 2);
        game_state->viewport.width  = pixel_width;
        game_state->viewport.height = pixel_height;

        glViewport(
            game_state->viewport.left,
            game_state->viewport.bottom,
            game_state->viewport.width,
            game_state->viewport.height
        );
    }

    game_state->unit_to_pixels = (f32)game_state->viewport.width / (f32)VIRTUAL_WINDOW_W;
    game_state->pixels_to_units = 1.0f / game_state->unit_to_pixels;

    recalculate_tile_grid(game_state->grid_cols, game_state->grid_rows);
}

internal i32 _compare_fov_entries(const void *a, const void *b) {
    FOVPointEntry *entry_a = (FOVPointEntry *)a;
    FOVPointEntry *entry_b = (FOVPointEntry *)b;

    // We are doing from highest to lowest
    if (entry_a->angle < entry_b->angle) return -1;
    if (entry_a->angle > entry_b->angle) return  1;
    return 0;
}

internal void do_field_of_view_edge_corner(Vector2 from, Vector2 corner, Array<FOVPointEntry> &fov_points) {
    const f32 DIR_OFFSET_RADIANS = 0.0001f; // Offset from corner in each direction

    // We cast 3 rays, one directly on the corner, one slightly to the right and one slightly to the left.
    // This allows us to create full field of view mask.
    Vector2 dirs[3];
    dirs[0] = direction(from, corner);
    dirs[1] = rotated(dirs[0],  DIR_OFFSET_RADIANS);
    dirs[2] = rotated(dirs[0], -DIR_OFFSET_RADIANS);

    f32 angles[3];
    angles[0] = angle(V2_UP, dirs[0]);
    angles[1] = angles[0] + DIR_OFFSET_RADIANS;
    angles[2] = angles[0] - DIR_OFFSET_RADIANS;

    Rect tile_area = game_state->tile_area;

    for (u32 ray_i = 0; ray_i < 3; ray_i++) {
        FOVPointEntry *fov_entry = fov_points.allocate_item();

        fov_entry->angle = angles[ray_i];

        HitData hit;
        bool did_hit = do_field_of_view_raycast(from, dirs[ray_i], hit);

        if (did_hit) {
            fov_entry->vertex = from + (dirs[ray_i] * hit.hit_distance);
        }
        else {
            Vector2 distant_point = from + (dirs[ray_i] * 100.0f);
            Vector2 *level_corners = game_state->level_corners;
            Edge to_distant_point = { from, distant_point };
            bool intersection_found = false;

            Edge edges[] = {
                { level_corners[2], level_corners[3] }, // UP
                { level_corners[1], level_corners[2] }, // RIGHT
                { level_corners[0], level_corners[1] }, // DOWN
                { level_corners[3], level_corners[0] }  // LEFT
            };

            for (u32 i = 0; i < 4; i++) {
                Vector2 intersection;
                bool has_intersected = line_segment_intersection(to_distant_point, edges[i], &intersection);

                if (has_intersected) {
                    fov_entry->vertex = intersection;
                    break;
                }
            }

            // Line has to always intersect with atleast the level edge!!
            assert(intersection_found);
        }
    }
}

internal void tick() {
    Entity *entity;
    bucket_array_foreach(game_state->entities.base_entities, entity) {
        if (entity->flags.disabled) continue;

        assert(entity->power_group < POWER_GROUP_COUNT);

        if (!game_state->power_states[entity->power_group]) continue;

        switch (entity->type) {
            case EntityType::Belt : {
                Vector2i direction = entity_rotation_to_direction(entity->logical_rotation);
                Vector2i to = entity->grid_position + direction;

                TileState *to_move = search_lookup_grid(to);

                if (!to_move || to_move->top_layer.type != EntityType::None) {
                    try_move_block_recursive(entity->grid_position, direction, 0.7f);
                };
            } break;

            case EntityType::PushBlock : {
                Vector2i direction = entity_rotation_to_direction(entity->logical_rotation);

                Vector2i target_position = entity->grid_position + direction;
                target_position = wrap_level_position(target_position);

                TileState *target_tile = search_lookup_grid(target_position);

                f32 move_speed = 0.7f;

                bool is_target_tile_availible = target_tile && target_tile->top_layer.type == EntityType::None;

                if (is_target_tile_availible || try_move_block_recursive(target_position, direction, move_speed)) {
                    add_transaction(entity->id, entity->grid_position, target_position, move_speed);
                }
            } break;
        }
    }}
}

internal void update_and_render_game(f32 dt) {
    bool level_contains_goals = false;
    bool all_goals_reached = true;

    // Check if all goals are reached
    {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            if (entity->type == EntityType::Goal) {
                level_contains_goals = true;

                TileState *tile = search_lookup_grid(entity->grid_position);

                if (tile->top_layer.type != EntityType::BasicBlock) {
                    all_goals_reached = false;
                };
            }
            else if (entity->type == EntityType::PlayerGoal) {
                level_contains_goals = true;

                TileState *tile = search_lookup_grid(entity->grid_position);

                if (tile->top_layer.type != EntityType::Player) {
                    all_goals_reached = false;
                };
            }
        }}

        if (game_state->current_level_index >= 0 && level_contains_goals && all_goals_reached) {
            game_state->levels_solved_state[game_state->current_level_index] = true;

            if (game_state->last_overworld_index >= 0) {
                save_game_state(game_state);

                const char* overworld_name = GAME_LEVELS[game_state->last_overworld_index].name;
                load_level(overworld_name);
                return;
            }
        }
    }

    // Player blink effect
    {
        if (game_state->player_blink_effect > 0.0f) {
            game_state->player_blink_effect -= dt;

            if (game_state->player_blink_effect < 0.0f) game_state->player_blink_effect = 0.0f;
        }
    }

    // Do player movement
    {
        if (game_state->player_last_input <= PLAYER_INPUT_DELAY) {
            game_state->player_last_input += dt;
        }

        if (game_state->player_move_commands.length > 0) {
            game_state->player_blink_effect = 0.0f;     // Cancel blinking

            i32 players_in_transaction = 0;

            // Find all players that are still moving
            {
                Entity *entity;
                bucket_array_foreach(game_state->entities.base_entities, entity) {
                    if (entity->type != EntityType::Player) continue;

                    if (is_in_transaction(entity->id)) players_in_transaction += 1;
                }}
            }

            // If all finished their movement, try to execute another move command
            if (players_in_transaction == 0) {
                Vector2i &direction = game_state->player_move_commands[0];

                Entity *entity;
                bucket_array_foreach(game_state->entities.base_entities, entity) {
                    if (entity->type != EntityType::Player) continue;

                    entity->logical_rotation = entity_direction_to_rotation(direction);

                    Vector2i player_final_position = entity->grid_position + direction;
                    player_final_position = wrap_level_position(player_final_position);

                    TileState *target_state = search_lookup_grid(player_final_position);

                    if (!target_state) continue;

                    if (target_state->top_layer.type == EntityType::None) {
                        add_transaction(entity->id, entity->grid_position, player_final_position, PLAYER_SPEED);
                    }
                    else {  // Try to move the block
                        Entity *block = &game_state->entities.base_entities[target_state->top_layer.id.location];
                        if (block->flags.unmovable) continue;

                        Vector2i block_final = player_final_position + direction;
                        block_final = wrap_level_position(block_final);

                        TileState *block_target = search_lookup_grid(block_final);

                        // Allow to move only one block (if another is blocking, ignore)
                        if (block_target && block_target->top_layer.type == EntityType::None) {
                            add_transaction(entity->id, entity->grid_position, player_final_position, PLAYER_SPEED);
                            add_transaction(block->id, player_final_position, block_final, PLAYER_SPEED);
                        }
                    }
                }}

                game_state->player_move_commands.remove(0);
            }
        }
    }

    // Reset states
    {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            TileState *tile = search_lookup_grid(entity->grid_position);

            if (entity->logic_level == EntityLogicLayer::Top && tile->top_layer.type == EntityType::None) {
                entity->flags.disabled = false;
            }
            else if (entity->logic_level == EntityLogicLayer::Bottom && tile->bottom_layer.type == EntityType::None) {
                entity->flags.disabled = false;
            }
        }}
    }

    update_lookup_grid();

    // Reset power states
    for (u32 index = 0; index < POWER_GROUP_COUNT; index++) {
        game_state->power_states[index] = false;
    }

    bool is_any_power_button_pressed = false;

    // Checck if power is on
    {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            if (entity->type == EntityType::Button) {
                TileState *tile = search_lookup_grid(entity->grid_position);

                if (tile && tile->top_layer.type != EntityType::None) {
                    Entity *button = &game_state->entities.base_entities[tile->top_layer.id.location];

                    assert(button->power_group < POWER_GROUP_COUNT);

                    if (button->power_group >= POWER_GROUP_COUNT) {
                        log_print("Invalid power group on a button (%d)!\n", button->power_group);
                        button->power_group = 0;
                    }

                    game_state->power_states[entity->power_group] = true;
                    is_any_power_button_pressed = true;
                }
            }
        }}
    }

    // Update electric walls
    {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            if (entity->type == EntityType::PoweredWall) {
                bool is_power_on = game_state->power_states[entity->power_group];

                if (is_power_on) {
                    entity->flags.disabled = true;
                }
                else {
                    if (entity->flags.disabled) {
                        TileState *tile = search_lookup_grid(entity->grid_position);

                        if (tile && tile->top_layer.type == EntityType::None) {
                            entity->flags.disabled = false;
                        }
                    }
                }
            }
        }}
    }

    // Negator stuff
    {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            if (entity->type == EntityType::Negator) {
                // For each direction, proagate the negation to the same block type
                for (u32 i = 0; i < 4; ++i) {
                    Vector2i direction = get_direction_from_logical_rotation(i);

                    Vector2i start_tile_position = entity->grid_position + direction;
                    start_tile_position = wrap_level_position(start_tile_position);

                    propagate_negation(start_tile_position);
                }
            }
        }}
    }

    // Update entities (that use realtime not ticks)
    {
        Entity *entity;
        bucket_array_foreach(game_state->entities.base_entities, entity) {
            if (entity->flags.disabled) continue;

            if (entity->flags.is_powered) {
                assert(entity->power_group < POWER_GROUP_COUNT);

                entity->override_color = POWER_GROUP_COLORS[entity->power_group];
            }
            else {
                entity->override_color = Color_WHITE;
            }

            switch (entity->type) {
                case EntityType::Piston : {
                    Vector2i direction = entity_rotation_to_direction(entity->logical_rotation);
                    Vector2i from = entity->grid_position + direction;
                    try_move_block_recursive(from, direction, 0.7f);
                } break;

                case EntityType::Rotator : {
                    TileState *tile = search_lookup_grid(entity->grid_position);
                    assert(tile != nullptr);

                    EntityRotator *data = &game_state->entities.entity_data[entity->data_location].rotator;

                    if (tile->top_layer.type == EntityType::None || tile->top_layer.type == EntityType::Player) {
                        data->was_last_update_empty = true;
                    }
                    else if (data->was_last_update_empty || !are_entity_ids_equal(data->last_rotated_id, tile->top_layer.id)) {
                        Entity *to_rotate = &game_state->entities.base_entities[tile->top_layer.id.location];

                        entity_rotate_clockwise(to_rotate);

                        data->last_rotated_id = to_rotate->id;
                        data->was_last_update_empty = false;
                    }
                } break;

                // @Todo: recalc edges with each player move.
                case EntityType::Player : {
                    f32 effect = (sin((game_state->player_blink_effect + (0.175f * PI)) * 20.0f) + 1) * 0.5f;
                    u8 opacity = 255 - (u8)(255.0f * effect);
                    entity->override_color.r = 255;
                    entity->override_color.g = 255;
                    entity->override_color.b = 255;
                    entity->override_color.a = opacity;

                    if (game_state->do_fov_fog) {
                        Vector2 player_pos = entity->visual_position;

                        Array<FOVPointEntry> fov_hit_points;
                        allocate_array(fov_hit_points, 512);

                        for (u32 i = 0; i < 4; i++) {
                            FOVPointEntry *fov_entry = fov_hit_points.allocate_item();
                            Vector2 dir = direction(player_pos, game_state->level_corners[i]);

                            fov_entry->angle = angle(V2_UP, dir);

                            HitData hit;
                            bool did_hit = do_field_of_view_raycast(player_pos, dir, hit);

                            if (did_hit) {
                                fov_entry->vertex = player_pos + (dir * hit.hit_distance);
                            }
                            else {
                                fov_entry->vertex = game_state->level_corners[i];
                            }
                        }

                        Edge *edge_it;
                        array_foreach(game_state->edges, edge_it) {
                            do_field_of_view_edge_corner(player_pos, edge_it->a, fov_hit_points);
                            do_field_of_view_edge_corner(player_pos, edge_it->b, fov_hit_points);
                        }

                        qsort(fov_hit_points.data, fov_hit_points.length, sizeof(fov_hit_points[0]), _compare_fov_entries);

                        StaticArray<Vector2> &fov_mesh = game_state->fov_mesh;
                        fov_mesh.clear();

                        fov_mesh.add(player_pos);
                        for (u32 i = 0; i < fov_hit_points.length; i++) {
                            fov_mesh.add(fov_hit_points[i].vertex);
                        }
                        fov_mesh.add(fov_mesh[1]);  // To finish up the loop (connect last and first).

                        free_array(fov_hit_points);
                    }
                } break;

                case EntityType::LevelTransition : {
                    TileState *tile = search_lookup_grid(entity->grid_position);

                    EntityLevelTransition *transition = &game_state->entities.entity_data[entity->data_location].level_transition;

                    if (transition->next_level_index >= 0) {
                        bool solved = game_state->levels_solved_state[transition->next_level_index];

                        if (solved) {
                            entity->override_color = Color_GREEN;
                        }
                        else {
                            entity->override_color = Color_WHITE;
                        }
                    }

                    if (tile->top_layer.type == EntityType::Player) {
                        Entity *player = &game_state->entities.base_entities[tile->top_layer.id.location];

                        if (player->logical_rotation == entity->logical_rotation && transition->next_level_index >= 0) {
                            const Level *level_to_load = &GAME_LEVELS[transition->next_level_index];

                            if (transition->is_to_puzzle) {
                                game_state->overworld_last_position.x = entity->grid_position.x;
                                game_state->overworld_last_position.y = entity->grid_position.y;

                                game_state->last_player_rotation = (player->logical_rotation + 2) % 4;

                                bool load_success = load_level(level_to_load->name);
                                assert_m(load_success, "Invalid puzzle level in transition tile!");
                            }
                            else if (transition->transition_link >= 0) {
                                i32 link = transition->transition_link;

                                game_state->last_player_rotation = player->logical_rotation;

                                bool load_success = load_level(level_to_load->name, link);
                                assert_m(load_success, "Invalid level in transition tile!");
                            }

                            if (level_to_load->type == LevelType::Overworld) {
                                save_game_state(game_state);
                            }

                            return;
                        }
                    }
                } break;

                case EntityType::BasicBlock : {
                    TileState *tile = search_lookup_grid(entity->grid_position);

                    if (tile && tile->bottom_layer.type == EntityType::Goal) {
                        entity->override_color = Color_GREEN;
                    }
                    else {
                        entity->override_color = Color_WHITE;
                    }
                } break;
            }
        }}
    }

    // Do ingame ticking
    {
        if (is_any_power_button_pressed) {
            game_state->game_tick_timer += dt;

            if (game_state->game_tick_timer >= GAME_TICK_FREQ) {
                tick();

                game_state->ticks_elapsed++;
                game_state->game_tick_timer -= GAME_TICK_FREQ;
            }
        }
        else {
            game_state->game_tick_timer = GAME_TICK_FREQ;
        }
    }

    // Update move transactions
    {
        MoveTransaction *it;
        array_foreach(game_state->move_transactions, it) {
            it->lerp_t = min(1.0f, it->lerp_t + (dt * MOVE_SPEED * it->speed_mult));

            if (it->lerp_t < 1.0f) {
                Entity *e = &game_state->entities.base_entities[it->entity_id.location];

                Vector2 from = grid_pos_to_real_pos(it->from.x, it->from.y);
                Vector2 to = grid_pos_to_real_pos(it->adjusted_to.x, it->adjusted_to.y);

                f32 level_min_x = game_state->level_corners[0].x;
                f32 level_min_y = game_state->level_corners[0].y;

                f32 level_max_x = game_state->level_corners[2].x;
                f32 level_max_y = game_state->level_corners[2].y;

                e->visual_position = lerp(from, to, it->lerp_t);

                if (e->visual_position.x < level_min_x) {
                    e->visual_position.x = level_max_x - abs(level_min_x - e->visual_position.x);
                }
                else if (e->visual_position.x > level_max_x) {
                    e->visual_position.x = level_min_x + abs(level_max_x - e->visual_position.x);
                }

                if (e->visual_position.y < level_min_y) {
                    e->visual_position.y = level_max_y - abs(level_min_y - e->visual_position.y);
                }
                else if (e->visual_position.y > level_max_y) {
                    e->visual_position.y = level_min_y + abs(level_min_y - e->visual_position.y);
                }
            };
        }
    }

    remove_flagged_entities(game_state->entities);
    cleanup_transactions();
    update_lookup_grid();

    // Draw entities!
    {
        Entity *e;
        bucket_array_foreach(game_state->entities.base_entities, e) {
            if (
                e->flags.disabled ||
                e->flags.is_visible_only_in_editor ||
                e->type == EntityType::None
            ) continue;

            draw_entity(e);
        }}
    }
}

internal void game_handle_key_down(KeyInput *key) {
    if (game_state->player_last_input >= PLAYER_INPUT_DELAY) {
        switch (key->virtual_code) {
            case VK_UP :
            case 'W' : {
                game_state->player_move_commands.add(make_vector2i(0, 1));
                game_state->player_last_input = 0.0f;
            } break;

            case VK_LEFT :
            case 'A' : {
                game_state->player_move_commands.add(make_vector2i(-1, 0));
                game_state->player_last_input = 0.0f;
            } break;

            case VK_DOWN :
            case 'S' : {
                game_state->player_move_commands.add(make_vector2i(0, -1));
                game_state->player_last_input = 0.0f;
            } break;

            case VK_RIGHT :
            case 'D' : {
                game_state->player_move_commands.add(make_vector2i(1, 0));
                game_state->player_last_input = 0.0f;
            } break;

            case 'R' : {
#if defined(DEVELOPER)
                load_level(game_state->current_level);
#else
                const char *current_level_name = GAME_LEVELS[game_state->current_level_index].name;
                load_level(current_level_name);
#endif
            } break;

            case 'Q' : {
                bool is_in_overworld = GAME_LEVELS[game_state->current_level_index].type == LevelType::Overworld;

                if (!is_in_overworld && game_state->last_overworld_index >= 0) {
                    const char *last_overworld = GAME_LEVELS[game_state->last_overworld_index].name;
                    load_level(last_overworld);
                }
            } break;
        }
    }
}

internal void game_handle_left_mouse_button() {}

internal void game_handle_right_mouse_button() {}
