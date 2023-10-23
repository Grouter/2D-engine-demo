struct MoveTransaction {
    EntityID entity_id;
    f32 speed_mult;
    f32 lerp_t;
    Vector2i from;
    Vector2i to;
    Vector2i adjusted_to;
    bool32 is_wrapping;
};
