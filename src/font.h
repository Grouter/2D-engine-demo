#if !defined(FONT_H)
#define FONT_H

typedef stbtt_packedchar GlyphData;

const f32 FONT_SIZE_SMALL  = 18.0f;
const f32 FONT_SIZE_MEDIUM = 32.0f;
const f32 FONT_SIZE_BIG    = 128.0f;

struct GlyphRenderData {
    Vector2 uv_offset;
    Vector2 uv_scale;
    Vector2 world_scale;
    Vector2 offset;
    f32 x_advance;
};

struct Font {
    u32 resource_id;
    u32 texture;

    u32 texture_width;
    u32 texture_height;

    f32 size;   // Height in pixels

    u32 first_char;
    u32 char_count;
    GlyphData *glyphs;
};

#endif

