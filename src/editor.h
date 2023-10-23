#if !defined(EDITOR_H)
#define EDITOR_H

struct EditorData {
    bool tile_selected;
    TileState *selected_tile;
    Vector2i tile_selection_position;

    bool32 is_building = false;
    i32 selected_entity_spawn = 0;
    Entity *build_ghost_entity;

    char transition_level_buffer[MAX_LEVEL_NAME_LEN];
};

global EditorData editor_data = {};

const f32 EDITOR_WIDTH_RATIO = 0.15f;

#endif
