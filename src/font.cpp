internal Font load_font(const char *path, f32 size, u32 font_resource_id) {
    // @Todo: adjust bitmap size according to the font size
    const u32 bitmap_size = 1024;   // width and height of the font bitmap

    u8 *temp_bitmap = (u8 *)malloc(bitmap_size * bitmap_size);
    u8 *ttf_buffer  = (u8 *)calloc(1 << 20, 1);

    Font font = {};
    font.resource_id = font_resource_id;
    font.glyphs = (GlyphData *)calloc(sizeof(GlyphData), 96);
    font.texture_width = bitmap_size;
    font.texture_height = bitmap_size;
    font.size = size;
    font.first_char = 32;
    font.char_count = 96;

    FILE *font_file = fopen(path, "rb");

    if (!font_file) {
        log_print("Couldn't open font file %s\n", path);
        exit(1);
    }

    fread(ttf_buffer, 1, 1 << 20, font_file);

    stbtt_pack_context pc;

    stbtt_PackBegin(&pc, temp_bitmap, font.texture_width, font.texture_height, font.texture_width, 1, nullptr);
    stbtt_PackFontRange(&pc, ttf_buffer, 0, font.size, font.first_char, font.char_count, font.glyphs);
    stbtt_PackEnd(&pc);

    glGenTextures(1, &font.texture);
    glBindTexture(GL_TEXTURE_2D, font.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font.texture_width, font.texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(ttf_buffer);
    free(temp_bitmap);
    fclose(font_file);

    log_print("Loaded font: %s\n", path);

    return font;
}

internal f32 get_string_width_in_units(Font *font, const char *text, f32 scale_factor = 1.0f) {
    f32 pixel_width = 0.0f;

    GlyphData *glyph;

    while(*text) {
        u32 index = (u32)(*text) - font->first_char;

        glyph = &font->glyphs[index];

        pixel_width += glyph->xadvance * scale_factor;

        text += 1;
    }

    return (pixel_width * game_state->pixels_to_units);
}

internal f32 get_integer_width_in_units(Font *font, i32 number, f32 scale_factor = 1.0f) {
    static char buffer[32];

    itoa(number, buffer, 10);

    return get_string_width_in_units(font, buffer, scale_factor);
}

internal f32 get_char_width_in_units(Font *font, u8 c, f32 scale_factor = 1.0f) {
    i32 index = (i32)(c) - (i32)font->first_char;
    GlyphData *glyph = &font->glyphs[index];

    return (glyph->xadvance * scale_factor * game_state->pixels_to_units);
}

// @Speed
internal GlyphRenderData get_glyph(Font *font, char c) {
    if ((u32)c < font->first_char || (u32)c > (font->first_char + font->char_count)) {
        log_print("Unsupported char: %c\n", c);
        c = '?';
    }

    u32 index = (u32)c - font->first_char;
    GlyphData *raw = &font->glyphs[index];

    GlyphRenderData data = {};

    data.uv_offset.x = (f32)raw->x0 / font->texture_width;
    data.uv_offset.y = (f32)raw->y1 / font->texture_height; // y1 because font bitmap is inverted

    data.uv_scale.x = (f32)(raw->x1 - raw->x0) / font->texture_width;
    data.uv_scale.y = (f32)(raw->y1 - raw->y0) / font->texture_height * -1.0f; // -1 because font bitmap is inverted

    data.world_scale.x = (f32)(raw->x1 - raw->x0) * game_state->pixels_to_units;
    data.world_scale.y = (f32)(raw->y1 - raw->y0) * game_state->pixels_to_units;

    data.offset = make_vector2(raw->xoff, raw->yoff2);
    data.x_advance = raw->xadvance;

    return data;
}
