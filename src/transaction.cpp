internal bool is_in_transaction(EntityID id) {
    MoveTransaction *it;
    array_foreach(game_state->move_transactions, it) {
        if (are_entity_ids_equal(it->entity_id, id)) return true;
    }

    return false;
}

internal bool add_transaction(EntityID entity_id, Vector2i from, Vector2i to, f32 speed_mult = 1.0f) {
    {
        MoveTransaction *it;
        array_foreach(game_state->move_transactions, it) {
            if (are_entity_ids_equal(it->entity_id, entity_id)) return false;
            if (it->to == to) return false;
        }
    }

    MoveTransaction *t = game_state->move_transactions.allocate_item();

    t->speed_mult = speed_mult;
    t->lerp_t = 0.0f;
    t->entity_id = entity_id;
    t->from = from;
    t->to = wrap_level_position(to);

    {
        bool is_wrapping = (abs(from.x - t->to.x) > 1) || (abs(from.y - t->to.y) > 1);
        t->is_wrapping = is_wrapping;

        if (is_wrapping) {
            i32 adjusted_offset_x = sign(t->to.x - from.x) * -1;
            i32 adjusted_offset_y = sign(t->to.y - from.y) * -1;

            i32 final_distance = 0;

            if (adjusted_offset_x != 0) {
                final_distance = game_state->grid_cols - abs(t->to.x - from.x);
            }
            else {
                final_distance = game_state->grid_rows - abs(t->to.y - from.y);
            }

            t->adjusted_to.x = from.x + adjusted_offset_x * final_distance;
            t->adjusted_to.y = from.y + adjusted_offset_y * final_distance;
        }
        else {
            t->adjusted_to = t->to;
        }
    }

    return true;
}

internal void cleanup_transactions() {
    for (i32 i = (i32)game_state->move_transactions.length - 1; i >= 0; --i) {
        MoveTransaction *it = &game_state->move_transactions[i];

        if (!is_entity_id_valid(game_state->entities, it->entity_id)) {
            game_state->move_transactions.remove((size_t)i);
        }
        else if (it->lerp_t >= 1.0f) {
            // Commit the transaction and set the logical position
            Entity *e = &game_state->entities.base_entities[it->entity_id.location];
            sync_set_grid_position(*e, it->to.x, it->to.y);

            game_state->move_transactions.remove((size_t)i);
        }
    }
}
