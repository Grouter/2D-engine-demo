#if !defined(ENTITY_H)
#define ENTITY_H

#define SIDE_UP 0
#define SIDE_RIGHT 1
#define SIDE_DOWN 2
#define SIDE_LEFT 3

#define MOVE_SPEED 13.0f    // @Todo: remove this
#define PLAYER_SPEED 2.0f
#define BELT_ANIMATION_SPEED 10.0f

#define MAX_ENTITY_COUNT 1024

#define POWER_GROUP_COUNT 4

struct EntityID {
    u32 generation;
    BucketLocation location;
};

union EntityFlags {
    u32 raw = 0;

    struct {
        u8 disabled    : 1;
        u8 destroy     : 1;
        u8 unmovable   : 1;
        u8 has_data    : 1;
        u8 blocks_view : 1;
        u8 is_powered  : 1;
        u8 is_visible_only_in_editor: 1;
    };
};

enum struct EntityType {
    None,
    Base,

    Piston,
    Belt,
    Goal,
    PlayerGoal,
    Wall,
    Player,
    Rotator,
    BasicBlock,
    PushBlock,
    Button,
    LevelTransition,
    LevelStart,
    PoweredWall,
    Negator,

    Count,
};

const char* ENTITY_STRING_NAMES[(u32)EntityType::Count] = {
    "None",
    "Base",
    "Piston",
    "Belt",
    "Goal",
    "PlayerGoal",
    "Wall",
    "Player",
    "Rotator",
    "BasicBlock",
    "PushBlock",
    "Button",
    "LevelTransition",
    "LevelStart",
    "PoweredWall",
    "Negator",
};

const Color POWER_GROUP_COLORS[POWER_GROUP_COUNT] = {
    make_color(238, 238, 238),
    Color_RED,
    Color_GREEN,
    Color_BLUE,
};

enum struct EntityLogicLayer {
    Bottom,
    Top,
};

struct EntityPointer {
    EntityType type;
    EntityID id;
};

struct TileState {
    EntityPointer top_layer;
    EntityPointer bottom_layer;

    // Tracks if there is a free tile next to each face.
    // Only for view blocking tiles.
    u8 has_edge[4] = { 0, 0, 0, 0 };
};

struct Entity {
    EntityID id = {};

    EntityType  type;
    EntityFlags flags = {};

    u32 power_group = 0;    // Only for powered blocks

    f32 z_layer  = ZLayer_DEFAULT;
    EntityLogicLayer logic_level = EntityLogicLayer::Top;

    BucketLocation data_location;

    Vector2i grid_position    = {};
    Vector2  visual_position  = {};
    u32      logical_rotation = SIDE_UP;
    Vector2  scale            = make_vector2(1.0);

    Color override_color = Color_WHITE;
    Vector2i bitmap_atlas_position = {};
    TextureResource texture = TextureResource_MAIN;
};

struct EntityRotator {
    EntityID last_rotated_id;
    bool32 was_last_update_empty = true;
};

struct EntityLevelTransition {
    i32 next_level_index;
    i32 transition_link;
    bool32 is_to_puzzle;
};

struct EntityData {
    Entity *base;

    union {
        EntityRotator rotator;
        EntityLevelTransition level_transition;
    };
};

struct EntityStorage {
    BucketArray<Entity> base_entities;
    BucketArray<EntityData> entity_data;
};

inline bool is_entity_id_valid(EntityStorage &storage, EntityID &id) {
    return (
        id.location.bucket_index < storage.base_entities.buckets.length
        &&
        id.location.slot_index < storage.base_entities.bucket_size
        &&
        storage.base_entities[id.location].id.generation == id.generation
    );
}

inline bool are_entity_ids_equal(EntityID &a, EntityID &b) {
    return (
        a.location.slot_index == b.location.slot_index
        &&
        a.location.bucket_index == b.location.bucket_index
        &&
        a.generation == b.generation
    );
}

internal void allocate_entity_storage(EntityStorage &storage, MemoryArena *memory) {
    u32 bucket_size = 16;
    u32 bucket_count = 64;

    assert((bucket_size * bucket_count) == MAX_ENTITY_COUNT);

    allocate_bucket_array_from_block(memory, storage.base_entities, bucket_size, bucket_count);
    allocate_bucket_array_from_block(memory, storage.entity_data,   bucket_size, bucket_count);
}

// @Speed: use hashes
internal EntityType entity_type_from_string(const char * type) {
    for (u32 i = 0; i < (u32)EntityType::Count; i++) {
        if (strcmp(type, ENTITY_STRING_NAMES[i]) == 0) return (EntityType)i;
    }

    return EntityType::None;
}

internal EntityType entity_type_from_string(const char * type, size_t length) {
    for (u32 i = 0; i < (u32)EntityType::Count; i++) {
        size_t to_match_length = strlen(ENTITY_STRING_NAMES[i]);
        size_t check_size = max(to_match_length, length);

        if (strncmp(type, ENTITY_STRING_NAMES[i], check_size) == 0) return (EntityType)i;
    }

    return EntityType::None;
}

inline f32 entity_rotation_to_radians(u32 logical_rotation) {
    assert_m(logical_rotation <= 3, "Invalid logical rotation!");

    static const f32 ENTITY_ROTATION_TO_RADS[] = {
        0.0f,
        HALF_PI,
        PI,
        ONE_AND_HALF_PI
    };

    f32 result = ENTITY_ROTATION_TO_RADS[logical_rotation];

    return result;
}

inline Vector2i entity_rotation_to_direction(u32 logical_rotation) {
    assert_m(logical_rotation <= 3, "Invalid logical rotation!");

    static const Vector2i ENTITY_ROTATION_TO_DIRECTION[] = {
        V2i_UP,
        V2i_RIGHT,
        V2i_DOWN,
        V2i_LEFT
    };

    Vector2i result = ENTITY_ROTATION_TO_DIRECTION[logical_rotation];

    return result;
}

inline void entity_rotate_clockwise(Entity *entity) {
    entity->logical_rotation = (entity->logical_rotation + 1) % 4;
}

internal u32 entity_direction_to_rotation(Vector2i direction) {
    u32 result = 0;

    if (direction == V2i_UP)         result = SIDE_UP;
    else if (direction == V2i_RIGHT) result = SIDE_RIGHT;
    else if (direction == V2i_DOWN)  result = SIDE_DOWN;
    else if (direction == V2i_LEFT)  result = SIDE_LEFT;
    else invalid_code_path("Cannot convert provided direction to rotation!");

    return result;
}

#endif
