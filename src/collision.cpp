inline bool is_tile_empty(Vector2i target) {
    TileState *state = search_lookup_grid(target);

    return (
        state->bottom_layer.type == EntityType::None
        &&
        state->top_layer.type == EntityType::None
    );
}

internal bool is_point_in_rectangle(Vector2 point, Vector2 rect_center, Vector2 rect_size) {
    return (
        point.x >= (rect_center.x - rect_size.x * 0.5f) &&
        point.x <= (rect_center.x + rect_size.x * 0.5f) &&
        point.y >= (rect_center.y - rect_size.y * 0.5f) &&
        point.y <= (rect_center.y + rect_size.y * 0.5f)
    );
}

internal RaycastHit raycast_intersection(Vector2i start, Vector2i direction) {
    RaycastHit result = {};

    f32 closest = FLT_MAX;

    Entity *e;
    bucket_array_foreach(game_state->entities.base_entities, e) {
        Vector2i delta = e->grid_position - start;

        if (angle(direction, delta) != 0.0f) continue;

        f32 dst = distance(start, e->grid_position);

        if (dst == 0.0f) continue;

        if (dst < closest) {
            closest = dst;

            result.type = Intersect_ENTITY;
            result.position = e->grid_position;
            result.hit.entity = e;
        }
    }}

    return result;
}

internal bool do_field_of_view_raycast(Vector2 from, Vector2 direction, HitData &hit_info) {
    Vector2 from_adjusted = from - game_state->tile_area_offset;
    Vector2i grid_pos = real_pos_to_grid_pos(from);

    Vector2 delta_dist;
    delta_dist.x = (direction.x == 0.0f) ? 1e30f : abs(1.0f / direction.x);
    delta_dist.y = (direction.y == 0.0f) ? 1e30f : abs(1.0f / direction.y);

    Vector2 side_dist;
    Vector2i step;

    i32 side;

    if (direction.x < 0.0f) {
        step.x = -1;
        side_dist.x = (from_adjusted.x - (f32)grid_pos.x) * delta_dist.x;
    }
    else {
        step.x = 1;
        side_dist.x = ((f32)grid_pos.x + 1.0f - from_adjusted.x) * delta_dist.x;
    }

    if (direction.y < 0.0f) {
        step.y = -1;
        side_dist.y = (from_adjusted.y - (f32)grid_pos.y) * delta_dist.y;
    }
    else {
        step.y = 1;
        side_dist.y = ((f32)grid_pos.y + 1.0f - from_adjusted.y) * delta_dist.y;
    }

    const u8 SIDE_X = (step.x < 0.0f) ? SIDE_LEFT : SIDE_RIGHT;
    const u8 SIDE_Y = (step.y < 0.0f) ? SIDE_UP   : SIDE_DOWN;

    while (1) {
        if (side_dist.x < side_dist.y) {
            side_dist.x += delta_dist.x;
            grid_pos.x += step.x;
            side = SIDE_X;
        }
        else {
            side_dist.y += delta_dist.y;
            grid_pos.y += step.y;
            side = SIDE_Y;
        }

        if (is_valid_grid_position(grid_pos.x, grid_pos.y)) {
            TileState *tile = search_lookup_grid(grid_pos);

            if (tile == nullptr) return false;

            if (tile->top_layer.type == EntityType::Wall) {
                hit_info.side = side;
                hit_info.logical_position = grid_pos;

                if (side == SIDE_X) hit_info.hit_distance = (side_dist.x - delta_dist.x);
                else                hit_info.hit_distance = (side_dist.y - delta_dist.y);

                return true;
            }
        }
        else {
            return false;
        }
    }

    return false;
}
