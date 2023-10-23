internal bool is_valid_grid_position(i32 grid_x, i32 grid_y) {
    return (grid_x >= 0 && grid_x < game_state->grid_cols && grid_y >= 0 && grid_y < game_state->grid_rows);
}

internal u32 grid_pos_to_index(i32 grid_x, i32 grid_y) {
    return grid_y * game_state->grid_cols + grid_x;
}

internal Vector2 grid_pos_to_real_pos(i32 grid_x, i32 grid_y) {
    Vector2 result;

    result.x = (f32)grid_x * game_state->unit_tile_size + game_state->unit_tile_half_size;
    result.y = (f32)grid_y * game_state->unit_tile_size + game_state->unit_tile_half_size;

    result += game_state->tile_area_offset;

    return result;
}

internal Vector2i real_pos_to_grid_pos(Vector2 position) {
    Vector2i result;

    position -= game_state->tile_area_offset;

    result.x = (i32)(position.x / game_state->unit_tile_size);
    result.y = (i32)(position.y / game_state->unit_tile_size);

    return result;
}

internal void sync_set_grid_position(Entity &entity, i32 x, i32 y) {
    entity.grid_position.x = x;
    entity.grid_position.y = y;

    entity.visual_position = grid_pos_to_real_pos(x, y);
}

internal void sync_set_visual_position(Entity &entity, Vector2 position) {
    entity.visual_position = position;
    entity.grid_position = real_pos_to_grid_pos(position);
}

internal Vector2i wrap_level_position(Vector2i raw_position) {
    Vector2i result;

    result.x = raw_position.x;
    if (result.x >= game_state->grid_cols) result.x = result.x % game_state->grid_cols;
    else if (result.x < 0) result.x = game_state->grid_cols + result.x;

    result.y = raw_position.y;
    if (result.y >= game_state->grid_rows) result.y = result.y % game_state->grid_rows;
    else if (result.y < 0) result.y = game_state->grid_rows + result.y;

    return result;
}

internal TileState* search_lookup_grid(i32 col, i32 row) {
    if (is_valid_grid_position(col, row)) {
        u32 tile_index = grid_pos_to_index(col, row);
        return &game_state->lookup_grid[tile_index];
    }
    else return nullptr;
}

internal TileState* search_lookup_grid(Vector2i position) {
    return search_lookup_grid(position.x, position.y);
}

internal void update_lookup_grid() {
    // Reset the grid
    u32 tile_count = game_state->grid_rows * game_state->grid_cols;
    memset(game_state->lookup_grid, 0, tile_count * sizeof(TileState));

    Entity *e;

    // Populate the grid
    bucket_array_foreach(game_state->entities.base_entities, e) {
        if (e->flags.disabled) continue;

        u32 tile_index = grid_pos_to_index(e->grid_position.x, e->grid_position.y);
        TileState *state = &game_state->lookup_grid[tile_index];

        switch (e->logic_level) {
            case EntityLogicLayer::Bottom : {
                state->bottom_layer.type = e->type;
                state->bottom_layer.id = e->id;
            } break;

            case EntityLogicLayer::Top : {
                state->top_layer.type = e->type;
                state->top_layer.id = e->id;
            } break;
        }
    }}

    // Populate the edge data
    bucket_array_foreach(game_state->entities.base_entities, e) {
        if (e->flags.disabled || !e->flags.blocks_view) continue;

        u32 tile_index = grid_pos_to_index(e->grid_position.x, e->grid_position.y);
        TileState *current_state = &game_state->lookup_grid[tile_index];

        TileState *neighbors[4] = {
            search_lookup_grid(e->grid_position + V2i_UP),
            search_lookup_grid(e->grid_position + V2i_RIGHT),
            search_lookup_grid(e->grid_position + V2i_DOWN),
            search_lookup_grid(e->grid_position + V2i_LEFT)
        };

        for (u32 i = 0; i < 4; i++) {
            if (neighbors[i] == nullptr) {
                current_state->has_edge[i] = true;
                continue;
            }

            if (neighbors[i]->top_layer.type != EntityType::None) {
                Entity *n = &game_state->entities.base_entities[neighbors[i]->top_layer.id.location];   // Only top layer tiles can block view
                current_state->has_edge[i] = (n->flags.blocks_view == false);
            }
            else current_state->has_edge[i] = true;
        }
    }}
}

internal void recalculate_tile_grid(u32 cols, u32 rows) {
    game_state->grid_cols = cols;
    game_state->grid_rows = rows;

    f32 screen_unit_w = game_state->viewport.width * game_state->pixels_to_units;
    f32 screen_unit_h = game_state->viewport.height * game_state->pixels_to_units;

    // I tried a dynamic tile size, but the atlas mapping kept bleeding over when the tiles where too small.
    // The sizes of the tile were always a very small fraction above/below 1.0, so I decided to hardcode 1.0 as the definitive tile size.
    game_state->unit_tile_size = 1.0f;
    game_state->unit_tile_half_size = game_state->unit_tile_size * 0.5f;

#if 0
    //
    // This is the original code for the dynamic tile resizing.
    //

    f32 vert_tile_size = screen_unit_h / game_state->grid_rows;
    f32 hori_tile_size = screen_unit_w / game_state->grid_cols;

    game_state->unit_tile_size = min(vert_tile_size, hori_tile_size);

    // Round the tile size down, so in pixel size it is always a whole number.
    {
        f32 tile_pixel_size = floor(game_state->unit_tile_size * game_state->unit_to_pixels);
        game_state->unit_tile_size = tile_pixel_size / game_state->unit_to_pixels;
    }
#endif

    game_state->tile_area_size = make_vector2(
        game_state->unit_tile_size * game_state->grid_cols,
        game_state->unit_tile_size * game_state->grid_rows
    );

    game_state->tile_area_offset.x = (screen_unit_w - game_state->tile_area_size.x) * 0.5f;
    game_state->tile_area_offset.y = (screen_unit_h - game_state->tile_area_size.y) * 0.5f;

    game_state->tile_area.min = game_state->tile_area_offset;
    game_state->tile_area.max = game_state->tile_area.min + game_state->tile_area_size;

    {
        Vector2 *level_corners = game_state->level_corners;

        level_corners[0] = game_state->tile_area.min;

        level_corners[1] = game_state->tile_area.min;
        level_corners[1].x += game_state->tile_area_size.x;

        level_corners[2] = game_state->tile_area.max;

        level_corners[3] = game_state->tile_area.min;
        level_corners[3].y += game_state->tile_area_size.y;
    }
}

internal void compute_tiles_edges(StaticArray<Edge> &edges) {
    edges.clear();

    i32 grid_w = game_state->grid_cols;
    i32 grid_h = game_state->grid_rows;

    u32 tile_count = grid_w * grid_h;

    Array<TileEdgeIndicies> tile_edge_indicies;
    allocate_array(tile_edge_indicies, tile_count);

    // @Cleanup
    // Fake fill the array
    memset(tile_edge_indicies.data, -1, tile_count * sizeof(TileEdgeIndicies));
    tile_edge_indicies.length = tile_count;

    Vector2i grid_position = {};

    for (u32 i = 0; i < tile_count; i++) {
        TileState *tile = &game_state->lookup_grid[i];

        if (tile->top_layer.type != EntityType::None) {
            Entity *entity = &game_state->entities.base_entities[tile->top_layer.id.location];
            Vector2 entity_real_position = grid_pos_to_real_pos(grid_position.x, grid_position.y);

            if (entity->flags.blocks_view && !entity->flags.disabled) {
                // Top edge
                if (tile->has_edge[0]) {
                    i32 current_edge_index = -1;

                    if (grid_position.x > 0) {
                        TileEdgeIndicies *left_neighbor = &tile_edge_indicies[i - 1];
                        i32 left_neighbor_top_edge_index = left_neighbor->edge_indicies[0];

                        current_edge_index = left_neighbor_top_edge_index;
                    }

                    if (current_edge_index == -1) {
                        Edge *new_edge = edges.allocate_item();
                        new_edge->a.x = entity_real_position.x - game_state->unit_tile_half_size;
                        new_edge->a.y = entity_real_position.y + game_state->unit_tile_half_size;

                        new_edge->b = new_edge->a;

                        current_edge_index = (u32)edges.length - 1;
                    }

                    Edge *edge_to_lengthen = &edges[current_edge_index];
                    edge_to_lengthen->b.x += game_state->unit_tile_size;

                    tile_edge_indicies[i].edge_indicies[0] = current_edge_index;
                }

                // Right edge
                if (tile->has_edge[1]) {
                    i32 current_edge_index = -1;

                    if (grid_position.y > 0) {
                        TileEdgeIndicies *top_neighbor = &tile_edge_indicies[i - grid_w];
                        i32 top_neighbor_right_edge_index = top_neighbor->edge_indicies[1];

                        current_edge_index = top_neighbor_right_edge_index;
                    }

                    if (current_edge_index == -1) {
                        Edge *new_edge = edges.allocate_item();
                        new_edge->a.x = entity_real_position.x + game_state->unit_tile_half_size;
                        new_edge->a.y = entity_real_position.y - game_state->unit_tile_half_size;

                        new_edge->b = new_edge->a;

                        current_edge_index = (u32)edges.length - 1;
                    }

                    Edge *edge_to_lengthen = &edges[current_edge_index];
                    edge_to_lengthen->b.y += game_state->unit_tile_size;

                    tile_edge_indicies[i].edge_indicies[1] = current_edge_index;
                }

                // Down edge
                if (tile->has_edge[2]) {
                    i32 current_edge_index = -1;

                    if (grid_position.x > 0) {
                        TileEdgeIndicies *left_neighbor = &tile_edge_indicies[i - 1];
                        i32 left_neighbor_bottom_edge_index = left_neighbor->edge_indicies[2];

                        current_edge_index = left_neighbor_bottom_edge_index;
                    }

                    if (current_edge_index == -1) {
                        Edge *new_edge = edges.allocate_item();
                        new_edge->a.x = entity_real_position.x - game_state->unit_tile_half_size;
                        new_edge->a.y = entity_real_position.y - game_state->unit_tile_half_size;

                        new_edge->b = new_edge->a;

                        current_edge_index = (u32)edges.length - 1;
                    }

                    Edge *edge_to_lengthen = &edges[current_edge_index];
                    edge_to_lengthen->b.x += game_state->unit_tile_size;

                    tile_edge_indicies[i].edge_indicies[2] = current_edge_index;
                }

                // Left edge
                if (tile->has_edge[3]) {
                    i32 current_edge_index = -1;

                    if (grid_position.y > 0) {
                        TileEdgeIndicies *top_neighbor = &tile_edge_indicies[i - grid_w];
                        i32 top_neighbor_left_edge_index = top_neighbor->edge_indicies[3];

                        current_edge_index = top_neighbor_left_edge_index;
                    }

                    if (current_edge_index == -1) {
                        Edge *new_edge = edges.allocate_item();
                        new_edge->a.x = entity_real_position.x - game_state->unit_tile_half_size;
                        new_edge->a.y = entity_real_position.y - game_state->unit_tile_half_size;

                        new_edge->b = new_edge->a;

                        current_edge_index = (u32)edges.length - 1;
                    }

                    Edge *edge_to_lengthen = &edges[current_edge_index];
                    edge_to_lengthen->b.y += game_state->unit_tile_size;

                    tile_edge_indicies[i].edge_indicies[3] = current_edge_index;
                }
            }
        }

        grid_position.x += 1;

        if (grid_position.x >= grid_w) {
            grid_position.x = 0;
            grid_position.y += 1;
        }
    }

    log_print("Computed %d edges\n", edges.length);

    free_array(tile_edge_indicies);
}
