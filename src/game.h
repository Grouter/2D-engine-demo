#if !defined(GAME_H)
#define GAME_H

#define GAME_TICK_FREQ 0.5f
#define PLAYER_INPUT_DELAY 0.05f

#define MAX_GRID_ROWS 32
#define MAX_GRID_COLS 32

#define PLAYER_BLINK 2.0f

struct Viewport {
    i32 left, bottom;
    i32 width, height;
};

struct FOVPointEntry {
    f32 angle;
    Vector2 vertex;
};

struct TileEdgeIndicies {
    i32 edge_indicies[4] = { -1, -1, -1, -1 };
};

struct GameState {
    //
    // Window and rendering
    //
    u32 window_width_pixels;
    u32 window_height_pixels;

    Viewport viewport;  // In pixels

    f32 unit_to_pixels = 1.0f;
    f32 pixels_to_units = 1.0f;

    //
    // Memory
    //
    MemoryArena permanent_memory;   // For runtime initialized permanent arrays and structs
    TemporaryMemoryArena temporary_memory; // For memory, that lasts only one frame.

    //
    // Dev stuff
    //
    StaticArray<Command> commands;
    Console console;

    //
    // Gameplay
    //
    Camera camera;
    EntityStorage entities;

    f32 time_elapsed = 0.0f;
    f32 game_tick_timer = 0.0f;
    u32 ticks_elapsed = 0;

    f32 belt_animation = 0.0f;
    f32 player_last_input = 0.0f;

    StaticArray<MoveTransaction> move_transactions;
    StaticArray<Vector2i> player_move_commands;

    bool power_states[POWER_GROUP_COUNT] = {};

    f32 player_blink_effect = PLAYER_BLINK;
    i32 current_level_index = 0;
    i32 last_overworld_index = -1;
    u32 last_player_rotation;
    Vector2i overworld_last_position = make_vector2i(-1, -1);

    bool levels_solved_state[STATIC_ARRAY_CAPACITY(GAME_LEVELS)];

    char current_level[MAX_LEVEL_NAME_LEN];

    //
    // Tile data
    //
    i32 grid_rows = MAX_GRID_ROWS;
    i32 grid_cols = MAX_GRID_COLS;
    f32 unit_tile_size;
    f32 unit_tile_half_size;
    Vector2 tile_area_offset;   // Offset of the current level from 0,0 coordinate
    Vector2 tile_area_size;  // The area in units, that current level takes up
    Rect tile_area;
    TileState lookup_grid[MAX_GRID_ROWS * MAX_GRID_COLS];
    Vector2 level_corners[4];   // Position in units (BL, BR, TR, TL)

    //
    // Edges, "polygonization" and fog
    //
    bool do_fov_fog = false;
    StaticArray<Edge> edges;
    StaticArray<Vector2> fov_mesh;
    u32 fov_vao;
    u32 fov_vbo;
};

internal void game_handle_key_down(KeyInput *key);

#endif
