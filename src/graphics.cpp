internal bool load_program(const char *path, u32 &handle, u32 input_count = 0, char **inputs = nullptr) {
    Array<char> shader_source;

    bool success = read_whole_file(path, shader_source);
    if (!success) return false;

    i32 compile_status;
    static char compile_log[512];

    u32 vertex_handle;
    u32 fragment_handle;

    Array<const char *> shader_inputs;
    allocate_array(shader_inputs, 8);

    shader_inputs.add(GLSL_VERSION_LINE);
    for (u32 i = 0; i < input_count; i++) shader_inputs.add(inputs[i]);

    // Compile the vertex part
    {
        vertex_handle = glCreateShader(GL_VERTEX_SHADER);

        shader_inputs.add("#define VERTEX\n");
        shader_inputs.add(shader_source.data);

        glShaderSource(vertex_handle, (i32)shader_inputs.length, shader_inputs.data, NULL);
        glCompileShader(vertex_handle);

        shader_inputs.length -= 2;

        SHADER_COMPILATION_CHECK(vertex_handle, compile_status, compile_log, 512, "Vertex");

        if (!compile_status) {
            glDeleteShader(vertex_handle);
            free_array(shader_source);
            free_array(shader_inputs);
            return false;
        }
    }

    // Compile the fragment part
    {
        fragment_handle = glCreateShader(GL_FRAGMENT_SHADER);

        shader_inputs.add("#define FRAGMENT\n");
        shader_inputs.add(shader_source.data);

        glShaderSource(fragment_handle, (i32)shader_inputs.length, shader_inputs.data, NULL);
        glCompileShader(fragment_handle);

        shader_inputs.length -= 2;

        SHADER_COMPILATION_CHECK(fragment_handle, compile_status, compile_log, 512, "Fragment");

        if (!compile_status) {
            glDeleteShader(fragment_handle);
            free_array(shader_source);
            free_array(shader_inputs);
            return false;
        }
    }

    // Initialize and link
    {
        handle = glCreateProgram();
        glAttachShader(handle, vertex_handle);
        glAttachShader(handle, fragment_handle);

        glLinkProgram(handle);
    }

    // Check link status
    {
        glGetProgramiv(handle, GL_LINK_STATUS, &compile_status);

        if (!compile_status) {
            glGetProgramInfoLog(handle, 512, NULL, compile_log);

            log_print("Program link error!\n");
            log_print("%s\n", compile_log);

            glDeleteShader(vertex_handle);
            glDeleteShader(fragment_handle);

            glDeleteProgram(handle);

            free_array(shader_source);
            free_array(shader_inputs);

            return false;
        }
    }

    glDetachShader(handle, vertex_handle);
    glDetachShader(handle, fragment_handle);

    glDeleteShader(vertex_handle);
    glDeleteShader(fragment_handle);

    free_array(shader_source);
    free_array(shader_inputs);

    return true;
}

internal bool load_shader(const char *path, Program &shader, u32 input_count = 0, char **inputs = nullptr) {
    u32 handle;

    bool success = load_program(path, handle, input_count, inputs);
    if (!success) return false;

    // Initialize and link
    shader.file_hash = hash_string(path);
    shader.handle = handle;

    // Update and set shader inputs
    if (input_count > 0 && shader.shader_inputs.length == 0) {
        allocate_array(shader.shader_inputs, input_count);
        for (u32 i = 0; i < input_count; i++)
            shader.shader_inputs.add(copy_string(inputs[i]));
    }

    log_print("Loaded shader: %s\n", path);

    return true;
}

internal void bind_mesh_buffer_objects(Mesh &mesh) {
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    {   // Index data
        glGenBuffers(1, &mesh.ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indicies.length * sizeof(u32), mesh.indicies.data, GL_STATIC_DRAW);
    }

    {   // Vertex data
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.verticies.length * sizeof(Vector3), mesh.verticies.data, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
    }

    {   // UV data
        glGenBuffers(1, &mesh.tbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.tbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.uvs.length * sizeof(Vector2), mesh.uvs.data, GL_STATIC_DRAW);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

internal Mesh create_mesh_quad(MemoryArena *memory) {
    Mesh quad = {};

    allocate_array_from_block(quad.verticies, 4, memory);
    allocate_array_from_block(quad.uvs,       4, memory);
    allocate_array_from_block(quad.indicies,  6, memory);

    quad.verticies.add(make_vector3(-0.5f,  0.5f, 0.0f));
    quad.verticies.add(make_vector3( 0.5f,  0.5f, 0.0f));
    quad.verticies.add(make_vector3( 0.5f, -0.5f, 0.0f));
    quad.verticies.add(make_vector3(-0.5f, -0.5f, 0.0f));

    quad.uvs.add(make_vector2(0.0f, 1.0f));
    quad.uvs.add(make_vector2(1.0f, 1.0f));
    quad.uvs.add(make_vector2(1.0f, 0.0f));
    quad.uvs.add(make_vector2(0.0f, 0.0f));

    quad.indicies.add(1);
    quad.indicies.add(0);
    quad.indicies.add(3);
    quad.indicies.add(3);
    quad.indicies.add(2);
    quad.indicies.add(1);

    bind_mesh_buffer_objects(quad);

    return quad;
}
