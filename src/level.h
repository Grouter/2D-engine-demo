#if !defined(LEVEL_H)
#define LEVEL_H

enum struct LevelType {
    None,
    Overworld,
    Puzzle,
    Other
};

struct Level {
    const char *name;
    LevelType type;
};

const Level GAME_LEVELS[] = {
    { "intro0",         LevelType::Other },
    { "intro1",         LevelType::Other },
    { "intro2",         LevelType::Other },
    { "intro3",         LevelType::Other },
    { "intro4",         LevelType::Other },
    { "intro5",         LevelType::Other },
    { "overworld1",     LevelType::Overworld },
    { "basic0",         LevelType::Puzzle },
    { "basic1",         LevelType::Puzzle },
    { "basic2",         LevelType::Puzzle },
    { "basic3",         LevelType::Puzzle },
    { "door",           LevelType::Puzzle },
    { "stack_pop",      LevelType::Puzzle },
    { "arcade_crane",   LevelType::Puzzle },
    { "timer",          LevelType::Puzzle },
    { "timer_exact",    LevelType::Puzzle },
    { "s_life",         LevelType::Other },
    { "overworld2",     LevelType::Overworld },
    { "barrier",        LevelType::Puzzle },
    { "wall_poof_tut",  LevelType::Puzzle },
    { "basic2_poof",    LevelType::Puzzle },
    { "cross_center",   LevelType::Puzzle },
    { "w_basic2",       LevelType::Puzzle },
    { "memory",         LevelType::Puzzle },
};

global u64 _GAME_LEVEL_HASHES[STATIC_ARRAY_CAPACITY(GAME_LEVELS)];

internal const char* get_level_name(u64 hash, u32 *index = nullptr) {
    u32 level_count = STATIC_ARRAY_CAPACITY(GAME_LEVELS);

    const char *result = nullptr;

    for (u32 i = 0; i < level_count; i++) {
        if (hash == _GAME_LEVEL_HASHES[i]) {
            if (index != nullptr) *index = i;

            result = GAME_LEVELS[i].name;

            break;
        }
    }

    return result;
}

internal Level get_level_data(const char *name, i32 *index = nullptr) {
    i32 level_count = STATIC_ARRAY_CAPACITY(GAME_LEVELS);
    u64 hash = hash_string(name);

    Level result;
    result.type = LevelType::None;
    result.name = nullptr;

    for (i32 i = 0; i < level_count; i++) {
        if (hash == _GAME_LEVEL_HASHES[i]) {
            result = GAME_LEVELS[i];
            if (index) *index = i;
            break;
        }
    }

    return result;
}

internal i32 get_level_index(const char *name) {
    u32 level_count = STATIC_ARRAY_CAPACITY(GAME_LEVELS);
    u64 hash = hash_string(name);

    i32 result = -1;

    for (u32 i = 0; i < level_count; i++) {
        if (hash == _GAME_LEVEL_HASHES[i]) {
            result = (i32)i;
            break;
        }
    }

    return result;
}

internal i32 get_level_index(const char *name, i32 length) {
    u32 level_count = STATIC_ARRAY_CAPACITY(GAME_LEVELS);
    u64 hash = hash_string(name, length);

    i32 result = -1;

    for (u32 i = 0; i < level_count; i++) {
        if (hash == _GAME_LEVEL_HASHES[i]) {
            result = (i32)i;
            break;
        }
    }

    return result;
}

internal i32 get_level_index(u64 level_name_hash) {
    u32 level_count = STATIC_ARRAY_CAPACITY(GAME_LEVELS);

    i32 result = -1;

    for (u32 i = 0; i < level_count; i++) {
        if (level_name_hash == _GAME_LEVEL_HASHES[i]) {
            result = (i32)i;
            break;
        }
    }

    return result;
}

#endif
