internal void draw_entities_for_editor(EntityStorage *storage) {
    Entity *entity;
    bucket_array_foreach(storage->base_entities, entity) {
        if (entity->flags.disabled || entity->type == EntityType::None) continue;

        if (entity->flags.is_powered) {
            assert(entity->power_group < POWER_GROUP_COUNT);

            entity->override_color = POWER_GROUP_COLORS[entity->power_group];
        }
        else {
            entity->override_color = Color_WHITE;
        }

        draw_entity(entity);
    }}
}

internal void flag_remove_tile(EntityStorage *storage, TileState *tile, EntityLogicLayer layer_to_remove) {
    Entity *to_destroy = nullptr;

    if (layer_to_remove == EntityLogicLayer::Top) {
        to_destroy = &storage->base_entities[tile->top_layer.id.location];
    }
    else {
        to_destroy = &storage->base_entities[tile->bottom_layer.id.location];
    }

    if (!to_destroy) return;

    to_destroy->flags.destroy = true;
}

// @Todo: this only removes one layer (the first not-None layer)!!!
// @Todo: this only removes one layer (the first not-None layer)!!!
// @Todo: this only removes one layer (the first not-None layer)!!!
// @Todo: this only removes one layer (the first not-None layer)!!!
internal void flag_remove_tile(EntityStorage *storage, TileState *tile) {
    Entity *to_destroy = nullptr;

    if (tile->top_layer.type != EntityType::None) {
        to_destroy = &storage->base_entities[tile->top_layer.id.location];
    }
    else if (tile->bottom_layer.type != EntityType::None) {
        to_destroy = &storage->base_entities[tile->bottom_layer.id.location];
    }

    if (!to_destroy) return;

    to_destroy->flags.destroy = true;
}

internal void flag_remove_tile(EntityStorage *storage, Vector2i position) {
    TileState *tile = search_lookup_grid(position);

    flag_remove_tile(storage, tile);
}

internal void remove_flagged_entities(EntityStorage &storage) {
    u32 removed = 0;

    Entity *e;
    bucket_array_foreach(storage.base_entities, e) {
        if (!e->flags.destroy) continue;

        if (e->flags.has_data) {
            storage.entity_data.remove(e->data_location);
        }

        e->id.generation += 1;
        storage.base_entities.remove(e->id.location);

        removed += 1;
    }}

    // if (removed > 0) log_print("Removed: %d entities\n", removed);
}

internal Entity* create_base_entity(EntityStorage *storage, EntityType type = EntityType::Base) {
    Entity *base;
    BucketLocation location = storage->base_entities.get_new(&base);

    u32 generation = base->id.generation;

    *base = {};
    base->id.location = location;
    base->id.generation = generation;
    base->z_layer = ZLayer_DEFAULT;
    base->type = type;
    base->scale = make_vector2(game_state->unit_tile_size);

    return base;
}

internal EntityData* create_entity_data(EntityStorage *storage, Entity &base) {
    EntityData *entity_data;
    BucketLocation location = storage->entity_data.get_new(&entity_data);

    entity_data->base = &base;

    base.data_location = location;
    base.flags.has_data = true;

    return entity_data;
}

internal Entity* create_piston(EntityStorage *storage) {
    Entity *piston = create_base_entity(storage, EntityType::Piston);

    piston->bitmap_atlas_position.x = 0;
    piston->bitmap_atlas_position.y = 0;

    return piston;
}

internal Entity* create_belt(EntityStorage *storage) {
    Entity *belt = create_base_entity(storage, EntityType::Belt);
    belt->z_layer = ZLayer_BOTTOM;
    belt->logic_level = EntityLogicLayer::Bottom;
    belt->flags.is_powered = 1;

    belt->bitmap_atlas_position.x = 4;
    belt->bitmap_atlas_position.y = 0;

    return belt;
}

internal Entity* create_goal(EntityStorage *storage) {
    Entity *goal = create_base_entity(storage, EntityType::Goal);
    goal->z_layer = ZLayer_BOTTOM;
    goal->logic_level = EntityLogicLayer::Bottom;

    goal->bitmap_atlas_position.x = 6;
    goal->bitmap_atlas_position.y = 0;

    return goal;
}

internal Entity* create_player_goal(EntityStorage *storage) {
    Entity *goal = create_base_entity(storage, EntityType::PlayerGoal);
    goal->z_layer = ZLayer_BOTTOM;
    goal->logic_level = EntityLogicLayer::Bottom;

    goal->bitmap_atlas_position.x = 9;
    goal->bitmap_atlas_position.y = 0;

    return goal;
}

internal Entity* create_wall(EntityStorage *storage) {
    Entity *wall = create_base_entity(storage, EntityType::Wall);
    wall->flags.unmovable = true;
    wall->flags.blocks_view = true;

    wall->bitmap_atlas_position.x = 2;
    wall->bitmap_atlas_position.y = 0;

    return wall;
}

internal Entity* create_player(EntityStorage *storage) {
    Entity *player = create_base_entity(storage, EntityType::Player);

    player->bitmap_atlas_position.x = 1;
    player->bitmap_atlas_position.y = 0;

    return player;
}

internal EntityData* create_rotator(EntityStorage *storage) {
    Entity *base = create_base_entity(storage, EntityType::Rotator);
    base->z_layer = ZLayer_BOTTOM;
    base->logic_level = EntityLogicLayer::Bottom;

    base->bitmap_atlas_position.x = 5;
    base->bitmap_atlas_position.y = 0;

    EntityData *data = create_entity_data(storage, *base);
    data->rotator = {};

    return data;
}

internal Entity* create_basic_block(EntityStorage *storage) {
    Entity *basic_block = create_base_entity(storage, EntityType::BasicBlock);

    basic_block->bitmap_atlas_position.x = 7;
    basic_block->bitmap_atlas_position.y = 0;

    return basic_block;
}

internal Entity* create_push_block(EntityStorage *storage) {
    Entity *push_block = create_base_entity(storage, EntityType::PushBlock);
    push_block->flags.is_powered = 1;

    push_block->bitmap_atlas_position.x = 12;
    push_block->bitmap_atlas_position.y = 0;

    return push_block;
}

internal Entity* create_button_entity(EntityStorage *storage) {
    Entity *button = create_base_entity(storage, EntityType::Button);
    button->z_layer = ZLayer_BOTTOM;
    button->logic_level = EntityLogicLayer::Bottom;
    button->flags.is_powered = 1;

    button->bitmap_atlas_position.x = 13;
    button->bitmap_atlas_position.y = 0;

    return button;
}

internal Entity* create_level_transition(EntityStorage *storage) {
    Entity *transition = create_base_entity(storage, EntityType::LevelTransition);
    transition->z_layer = ZLayer_BOTTOM;
    transition->logic_level = EntityLogicLayer::Bottom;

    transition->bitmap_atlas_position.x = 9;
    transition->bitmap_atlas_position.y = 1;

    EntityLevelTransition *data = &create_entity_data(storage, *transition)->level_transition;
    *data = {};


    return transition;
}

internal Entity* create_level_start(EntityStorage *storage) {
    Entity *transition = create_base_entity(storage, EntityType::LevelStart);
    transition->z_layer = ZLayer_BOTTOM;
    transition->logic_level = EntityLogicLayer::Bottom;

    transition->bitmap_atlas_position.x = 3;
    transition->bitmap_atlas_position.y = 0;

    return transition;
}

internal Entity* create_powered_wall(EntityStorage *storage) {
    Entity *wall = create_base_entity(storage, EntityType::PoweredWall);
    wall->flags.is_powered = 1;
    wall->flags.unmovable = true;
    wall->flags.blocks_view = true;

    wall->bitmap_atlas_position.x = 10;
    wall->bitmap_atlas_position.y = 1;

    return wall;
}

internal Entity* create_negator(EntityStorage *storage) {
    Entity *negator = create_base_entity(storage, EntityType::Negator);

    negator->bitmap_atlas_position.x = 10;
    negator->bitmap_atlas_position.y = 0;

    return negator;
}

internal Entity* create_entity_from_type(EntityStorage *storage, EntityType type) {
    switch (type) {
        case EntityType::None : { log_print("Trying to create a None entity!\n"); } break;
        case EntityType::Base : return create_base_entity(storage, type);
        case EntityType::Piston : return create_piston(storage);
        case EntityType::Belt : return create_belt(storage);
        case EntityType::Goal : return create_goal(storage);
        case EntityType::PlayerGoal : return create_player_goal(storage);
        case EntityType::Wall : return create_wall(storage);
        case EntityType::Player : return create_player(storage);
        case EntityType::Rotator : return create_rotator(storage)->base;
        case EntityType::BasicBlock : return create_basic_block(storage);
        case EntityType::PushBlock : return create_push_block(storage);
        case EntityType::Button : return create_button_entity(storage);
        case EntityType::LevelTransition : return create_level_transition(storage);
        case EntityType::LevelStart : return create_level_start(storage);
        case EntityType::PoweredWall : return create_powered_wall(storage);
        case EntityType::Negator : return create_negator(storage);

        default : {
            log_print("Unknown tile type in create_entity_from_type spawn\n");
        }
    }

    return nullptr;
}

internal bool entity_can_be_placed(Entity *entity, TileState *tile) {
    if (entity->logic_level == EntityLogicLayer::Bottom) {
        return tile->bottom_layer.type == EntityType::None;
    }
    else if (entity->logic_level == EntityLogicLayer::Top) {
        return tile->top_layer.type == EntityType::None;
    }

    return false;
}

// direction is:
// 0 -> UP
// 1 -> RIGHT
// 2 -> DOWN
// 3 -> LEFT
internal Vector2i get_direction_from_logical_rotation(u32 logical_rotation) {
    Vector2i direction = {};

    if (logical_rotation == 0)      ++direction.y;
    else if (logical_rotation == 1) ++direction.x;
    else if (logical_rotation == 2) --direction.y;
    else if (logical_rotation == 3) --direction.x;

    return direction;
}

// Returns false when blocks cannot be moved or there are no blocks to be moved.
internal bool try_move_block_recursive(Vector2i from, Vector2i direction, f32 move_speed = 1.0f) {
    Vector2i to = from + direction;

    to = wrap_level_position(to);

    TileState *block_to_move = search_lookup_grid(from);
    TileState *block_target = search_lookup_grid(to);

    if (!block_to_move || !block_target) return false;

    if (block_to_move->top_layer.type != EntityType::None) {
        Entity *block_entity = &game_state->entities.base_entities[block_to_move->top_layer.id.location];

        if (block_entity->flags.unmovable) return false;

        if (block_target->top_layer.type == EntityType::None || try_move_block_recursive(to, direction)) {
            add_transaction(block_to_move->top_layer.id, from, to, move_speed);

            return true;
        }

        return false;
    }

    return false;
}

internal bool can_source_see_target(Vector2i source, Vector2i target, bool ignore_player = true) {
    if (!(source.x == target.x || source.y == target.y)) return false;
    if (source == target) return true;

    Vector2i dir = target - source;
    dir.x = sign(dir.x);
    dir.y = sign(dir.y);

    Vector2i search_pos = source;

    while (1) {
        search_pos += dir;

        if (search_pos == target) return true;

        TileState *tile = search_lookup_grid(search_pos);
        if (!tile) return false;
        if (tile->top_layer.type == EntityType::Player) continue;
        if (tile->top_layer.type != EntityType::None) return false;
    }

    return false;
}

internal void propagate_negation(Vector2i start_position) {
    TileState *start_tile = search_lookup_grid(start_position);

    if (
        !start_tile ||
        start_tile->top_layer.type != EntityType::Wall
    ) return;

    Entity *to_negate = &game_state->entities.base_entities[start_tile->top_layer.id.location];

    if (to_negate->flags.disabled) return;

    to_negate->flags.disabled = true;

    for (u32 i = 0; i < 4; ++i) {
        Vector2i direction = get_direction_from_logical_rotation(i);

        Vector2i neighbor_position = start_position + direction;
        neighbor_position = wrap_level_position(neighbor_position);

        TileState *neighbor_tile = search_lookup_grid(neighbor_position);

        if (neighbor_tile && neighbor_tile->top_layer.type == start_tile->top_layer.type) {
            propagate_negation(neighbor_position);
        }
    }
}
