#if !defined(GRAPHICS_H)
#define GRAPHICS_H

#define SHADER_COMPILATION_CHECK(handle,status,log,log_size,log_type) {\
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);\
    if (!status) {\
        glGetShaderInfoLog(handle, log_size, NULL, log);\
        log_print("%s shader compilation failed:\n\n", log_type);\
        log_print("%s\n", log);\
    }\
}

const char* GLSL_VERSION = "#version 420";
const char* GLSL_VERSION_LINE = "#version 420\n";

struct Program {
    u32 handle;

    Array<char *> shader_inputs = {};
    u64 file_hash;
};

struct Texture {
    u32 handle;
    u32 pixel_width;
    u32 pixel_height;
};

struct Mesh {
    // OpenGL buffers
    u32 vao;
    u32 vbo;
    u32 ebo;
    u32 tbo;

    // Loaded data
    StaticArray<Vector3>     verticies;
    StaticArray<u32>         indicies;
    StaticArray<Vector2>     uvs;
};

#endif
