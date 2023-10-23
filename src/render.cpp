internal void _allocate_draw_call_buffer(DrawCallBuffer &buffer, u64 size, MemoryArena *memory) {
    Mesh *quad = &resources->meshes[MeshResource::MeshResource_QUAD];

    allocate_array_from_block(buffer.data, (size_t)size, memory);
    allocate_instance_buffer(quad, &buffer.vao, &buffer.instance_buffer, size);
}

internal void init_renderer(MemoryArena *memory) {
    _allocate_draw_call_buffer(render_state->draw_calls[TextureResource_WHITE], 1024, memory);
    _allocate_draw_call_buffer(render_state->draw_calls[TextureResource_MAIN],  1024, memory);

    _allocate_draw_call_buffer(render_state->font_draw_calls[FontResource_SMALL],  2048, memory);
    _allocate_draw_call_buffer(render_state->font_draw_calls[FontResource_MEDIUM], 1024, memory);
    _allocate_draw_call_buffer(render_state->font_draw_calls[FontResource_BIG],    256,  memory);

    allocate_line_instance_buffer(render_state->line_draw_calls);
}

inline void set_shader(ShaderResource shader) {
    Program *found = &resources->programs[shader];

    render_state->current_shader = found;

    glUseProgram(render_state->current_shader->handle);
}

inline void set_shader_int(const char *name, i32 value) {
    i32 loc = glGetUniformLocation(render_state->current_shader->handle, name);

    if (loc >= 0) {
        glUniform1i(loc, value);
    }
    else {
        log_print("Shader set int loc error! (attribute: %s)\n", name);
    }
}

inline void set_shader_diffuse_texture(u32 texture_handle) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    i32 loc = glGetUniformLocation(render_state->current_shader->handle, "diffuse_texture");

    if (loc >= 0) {
        glUniform1i(loc, 0);
    }
    else {
        log_print("Shader set diffuse texture loc error!\n");
    }
}

inline void set_shader_matrix4x4(const char *attr_name, Matrix4x4 value) {
    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attr_name);

    if (loc >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, value.raw);
    }
    else {
        log_print("Shader set Matrix4x4 loc error (attribute: %s)!\n", attr_name);
    }
}

inline void set_shader_vector4(const char *attr_name, Vector4 value) {
    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attr_name);

    if (loc >= 0) {
        glUniform4f(loc, value.r, value.g, value.b, value.a);
    }
    else {
        log_print("Shader set Vector4 loc error (attribute: %s)!\n", attr_name);
    }
}

internal void draw_rect(
    Vector2 position,
    Vector2 size,
    f32     rotation,
    Vector2 uv_offset,
    Vector2 uv_scale,
    TextureResource texture,
    f32     z_layer        = ZLayer_DEFAULT,
    Color   override_color = Color_WHITE
) {
    DrawCallData draw_call = {};

    draw_call.override_color = color_to_v4(override_color);

    draw_call.uv_offset = uv_offset;

    draw_call.transform = identity();
    scale(draw_call.transform, size.x, size.y, 1.0f);
    translate(draw_call.transform, position.x, position.y, z_layer);
    rotate(draw_call.transform, 0.0f, 0.0f, rotation);

    render_state->draw_calls[texture].data.add(draw_call);
}

internal void draw_rect(Vector2 position, Vector2 size, f32 rotation, f32 z_layer = ZLayer_DEFAULT, Color override_color = Color_WHITE) {
    DrawCallData draw_call = {};

    draw_call.override_color = color_to_v4(override_color);

    draw_call.transform = identity();
    scale(draw_call.transform, size.x, size.y, 1.0f);
    translate(draw_call.transform, position.x, position.y, z_layer);
    rotate(draw_call.transform, 0.0f, 0.0f, rotation);

    render_state->draw_calls[TextureResource_WHITE].data.add(draw_call);
}

internal void draw_rect(Vector2 position, Vector2 size, f32 rotation, f32 z_layer, Vector4 override_color) {
    DrawCallData draw_call = {};

    draw_call.override_color = override_color;

    draw_call.transform = identity();
    scale(draw_call.transform, size.x, size.y, 1.0f);
    translate(draw_call.transform, position.x, position.y, z_layer);
    rotate(draw_call.transform, 0.0f, 0.0f, rotation);

    render_state->draw_calls[TextureResource_WHITE].data.add(draw_call);
}

// Position = bottom left of the rendered text bounding box
internal void draw_text(Font *font, char *text, Vector3 position, Color color, f32 size_scale = 1.0f) {
    DrawCallBuffer *font_draw_calls = render_state->font_draw_calls;

    while (text[0] != '\0') {
        GlyphRenderData glyph = get_glyph(font, *text);

        if (*text == ' ') {
            // If space, just advance to the next character
            position.x += glyph.x_advance * size_scale * game_state->pixels_to_units;
            text += 1;
            continue;
        }

        DrawCallData draw_call = {};
        {
            draw_call.override_color = color_to_v4(color);
            draw_call.transform = identity();

            f32 x_offset = (glyph.world_scale.x * size_scale * 0.5f) + (glyph.offset.x * size_scale * game_state->pixels_to_units);
            f32 y_offset = (glyph.world_scale.y * size_scale * 0.5f) - (glyph.offset.y * size_scale * game_state->pixels_to_units);

            translate(draw_call.transform, position.x + x_offset, position.y + y_offset, position.z);

            scale(draw_call.transform, glyph.world_scale.x * size_scale, glyph.world_scale.y * size_scale, 1.0f);

            draw_call.uv_offset = glyph.uv_offset;
            draw_call.uv_scale = glyph.uv_scale;
        }

        if (font_draw_calls[font->resource_id].data.is_full()) {
            log_print("Exceeding font draw calls capacity!!!\n");
        }

        font_draw_calls[font->resource_id].data.add(draw_call);

        // Advance to the next character
        position.x += glyph.x_advance * size_scale * game_state->pixels_to_units;
        text += 1;
    }
}

internal void draw_register_value(Font *font, u32 value, Vector2 position, f32 z_layer, Color color, f32 size_scale = 1.0f) {
    DrawCallBuffer *font_draw_calls = render_state->font_draw_calls;

    char data_text = (char)('0' + value);
    GlyphRenderData glyph = get_glyph(font, data_text);

    DrawCallData draw_call = {};
    {
        draw_call.override_color = color_to_v4(color);

        draw_call.transform = identity();
        translate(draw_call.transform, position.x, position.y, z_layer);
        scale(draw_call.transform, glyph.world_scale.x * size_scale, glyph.world_scale.y * size_scale, 1.0f);

        draw_call.uv_offset = glyph.uv_offset;
        draw_call.uv_scale = glyph.uv_scale;
    }

    if (font_draw_calls[font->resource_id].data.is_full()) {
        log_print("Exceeding font draw calls capacity!!!\n");
    }

    font_draw_calls[font->resource_id].data.add(draw_call);
}

internal void draw_entity(Entity *entity) {
    DrawCallData data = {};

    data.override_color = color_to_v4(entity->override_color);
    data.transform = identity();
    scale(data.transform, entity->scale.x, entity->scale.y, 1.0f);
    translate(data.transform, entity->visual_position.x, entity->visual_position.y, entity->z_layer);

    f32 rotation_radians = entity_rotation_to_radians(entity->logical_rotation);
    rotate(data.transform, 0.0f, 0.0f, rotation_radians);

    get_main_atlas_uv_data(entity->bitmap_atlas_position.x, entity->bitmap_atlas_position.y, data.uv_offset, data.uv_scale);

    render_state->draw_calls[entity->texture].data.add(data);
}

// @Todo: add colors to lines
internal void draw_line(Vector2 start, Vector2 end, f32 z_layer = 0.0f, f32 thickness = 0.02f) {
    Vector2 dir = direction(start, end);

    Vector2 thickness_dir = make_vector2(dir.y, dir.x * -1.0f); // Create a perpendicular vector to the direction of the line
    thickness_dir = normalized(thickness_dir);

    Vector2 a = start - (thickness_dir * thickness);
    Vector2 b = start + (thickness_dir * thickness);
    Vector2 c = end   + (thickness_dir * thickness);
    Vector2 d = end   - (thickness_dir * thickness);

    LineDrawCallBuffer &buffer = render_state->line_draw_calls;

    LineDrawCallData *line_verticies = &buffer.data[buffer.data_length];
    line_verticies->verticies[0] = make_vector3(a, z_layer);
    line_verticies->verticies[1] = make_vector3(b, z_layer);
    line_verticies->verticies[2] = make_vector3(c, z_layer);

    line_verticies->verticies[3] = make_vector3(c, z_layer);
    line_verticies->verticies[4] = make_vector3(d, z_layer);
    line_verticies->verticies[5] = make_vector3(a, z_layer);

    buffer.data_length += 1;
}

internal i32 _comapre_draw_calls_for_sort(const void *a, const void *b) {
    DrawCallData *x = (DrawCallData *)a;
    DrawCallData *y = (DrawCallData *)b;

    if (x->override_color.a < y->override_color.a) return  1;
    if (x->override_color.a > y->override_color.a) return -1;
    return 0;
}

internal void flush_draw_calls() {
    //
    // Prepare and sort draw calls
    //
    for (u32 i = 0; i < TextureResource_COUNT; i++) {
        DrawCallBuffer &draw_calls = render_state->draw_calls[i];

        if (draw_calls.data.length == 0) continue;

        qsort(draw_calls.data.data, draw_calls.data.length, sizeof(DrawCallData), _comapre_draw_calls_for_sort);
        glNamedBufferSubData(draw_calls.instance_buffer, 0, draw_calls.data.length * sizeof(DrawCallData), draw_calls.data.data);

        draw_calls.first_transparent = -1;

        for (u32 j = 0; j < draw_calls.data.length; j++) {
            if (draw_calls.data[j].override_color.a != 1.0f) {
                draw_calls.first_transparent = j;
                break;
            }
        }
    }

    //
    // Opaques
    //
    glDepthMask(GL_TRUE);
    set_shader(ShaderResource::ShaderResource_DEFAULT);
    set_shader_matrix4x4("projection", game_state->camera.perspective);
    for (u32 i = 0; i < TextureResource_COUNT; i++) {
        DrawCallBuffer &draw_calls = render_state->draw_calls[i];

        if (draw_calls.data.length == 0) continue;
        if (draw_calls.first_transparent == 0) continue;

        set_shader_int("diffuse_alpha_mask", 0);
        set_shader_diffuse_texture(resources->textures[i].handle);

        glBindVertexArray(draw_calls.vao);

        if (draw_calls.first_transparent == -1) {
            glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, (i32)draw_calls.data.length, 0);
        }
        else if (draw_calls.first_transparent > 0) {
            glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, (i32)draw_calls.first_transparent, 0);
        }
    }

    //
    // Lines
    //
    if (render_state->line_draw_calls.data_length > 0) {
        glDepthMask(GL_TRUE);
        set_shader(ShaderResource::ShaderResource_LINE);
        set_shader_matrix4x4("projection", game_state->camera.perspective);

        LineDrawCallBuffer &buffer = render_state->line_draw_calls;

        glNamedBufferSubData(buffer.vbo, 0, buffer.data_length * sizeof(buffer.data[0]), buffer.data);

        glBindVertexArray(buffer.vao);

        glDrawArrays(GL_TRIANGLES, 0, (u32)buffer.data_length * 6);

        buffer.data_length = 0;
    }

    //
    // Transparent
    //
    glDepthMask(GL_FALSE);
    set_shader(ShaderResource::ShaderResource_DEFAULT);
    set_shader_matrix4x4("projection", game_state->camera.perspective);
    for (u32 i = 0; i < TextureResource_COUNT; i++) {
        DrawCallBuffer &draw_calls = render_state->draw_calls[i];

        if (draw_calls.data.length == 0) continue;
        if (draw_calls.first_transparent == -1) continue;

        set_shader_int("diffuse_alpha_mask", 0);
        set_shader_diffuse_texture(resources->textures[i].handle);

        glBindVertexArray(draw_calls.vao);

        u32 transparent_count = (u32)draw_calls.data.length - draw_calls.first_transparent;
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, (i32)transparent_count, draw_calls.first_transparent);
    }

    for (u32 i = 0; i < TextureResource_COUNT; i++) render_state->draw_calls[i].data.clear();

    //
    // FOV
    //
    // @Todo: this should be called from gameplay code
    if (program_state == ProgramState_GAMEPLAY && game_state->do_fov_fog) {
        glDepthMask(GL_FALSE);
        set_shader(ShaderResource::ShaderResource_FOV);
        set_shader_matrix4x4("projection", game_state->camera.perspective);

        u32 fov_vbo = game_state->fov_vbo;
        glNamedBufferSubData(fov_vbo, 0, array_bytes(game_state->fov_mesh), game_state->fov_mesh.data);

        glBindVertexArray(game_state->fov_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (u32)game_state->fov_mesh.length);
    }

    //
    // Fonts
    //
    glDepthMask(GL_TRUE);
    set_shader(ShaderResource::ShaderResource_DEFAULT);
    set_shader_matrix4x4("projection", game_state->camera.perspective);
    for (u32 i = 0; i < FontResource_COUNT; i++) {
        DrawCallBuffer &draw_calls = render_state->font_draw_calls[i];

        if (draw_calls.data.length == 0) continue;

        Font *font = get_font((FontResource)i);

        set_shader_int("diffuse_alpha_mask", 1);
        set_shader_vector4("material_color", V4_ONE);
        set_shader_diffuse_texture(font->texture);

        glNamedBufferSubData(draw_calls.instance_buffer, 0, draw_calls.data.length * sizeof(DrawCallData), draw_calls.data.data);

        glBindVertexArray(draw_calls.vao);

        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, (i32)draw_calls.data.length, 0);

        draw_calls.data.clear();
    }
}
