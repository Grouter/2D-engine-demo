inline Mesh *get_mesh(MeshResource mesh) {
    Mesh *result = &resources->meshes[mesh];

    return result;
}

inline Font* get_font(FontResource font_resource) {
    Font *font = &resources->fonts[font_resource];

    return font;
}

internal void get_main_atlas_uv_data(u32 col, u32 row, Vector2 &uv_offset, Vector2 &uv_scale) {
    Texture &atlas_texture = resources->textures[TextureResource_MAIN];

    u32 tile_pixel_size = 32;   // @Hardcode

    f32 cols = (f32)(atlas_texture.pixel_width / tile_pixel_size);
    f32 rows = (f32)(atlas_texture.pixel_height / tile_pixel_size);

    uv_offset.x = (f32)col / cols;
    uv_offset.y = (f32)row / rows;

    uv_scale.x = 1.0f / cols;
    uv_scale.y = 1.0f / rows;
}

internal Texture load_texture(const char *image) {
    std::string path = "textures/";
    path.append(image);

    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);

    i32 width, height, nr_channels;
    u8 *data = stbi_load(path.c_str(), &width, &height, &nr_channels, 0);

    if (data) {
        if (nr_channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (nr_channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else {
            log_print("Unsupported texture channels number\n");
        }
    }
    else {
        log_print("Error loading texture.\n");
    }

    stbi_image_free(data);

    Texture result;
    result.handle = texture;
    result.pixel_width = (u32)width;
    result.pixel_height = (u32)height;

    return result;
}

internal Texture create_white_texture() {
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    u8 data[4] = { 255, 255, 255, 255 };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    Texture result;
    result.handle = texture;
    result.pixel_width = 1;
    result.pixel_height = 1;

    return result;
}

internal void load_resources(Resources &res, MemoryArena *permanent_memory) {
    // Default shader
    {
        bool status = load_shader("shaders/default.glsl", res.programs[ShaderResource_DEFAULT]);
        if (!status) exit(1);
    }

    // Line shader
    {
        bool status = load_shader("shaders/line.glsl", res.programs[ShaderResource_LINE]);
        if (!status) exit(1);
    }

    // Fov shader
    {
        bool status = load_shader("shaders/fov.glsl", res.programs[ShaderResource_FOV]);
        if (!status) exit(1);
    }

    // Meshes
    res.meshes[MeshResource_QUAD] = create_mesh_quad(permanent_memory);

    // Textures
    res.textures[TextureResource_WHITE] = create_white_texture();
    res.textures[TextureResource_MAIN] = load_texture("atlas.png");

    // Fonts
    res.fonts[FontResource_SMALL]  = load_font("Merriweather-Regular.ttf", FONT_SIZE_SMALL, FontResource_SMALL);
    res.fonts[FontResource_MEDIUM] = load_font("Merriweather-Regular.ttf", FONT_SIZE_MEDIUM, FontResource_MEDIUM);
    res.fonts[FontResource_BIG]    = load_font("Merriweather-Regular.ttf", FONT_SIZE_BIG, FontResource_BIG);
}
