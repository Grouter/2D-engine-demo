#if !defined(RENDER_H)
#define RENDER_H

const f32 ZLayer_BEAM    = -0.11f;
const f32 ZLayer_BOTTOM  = -0.1f;
const f32 ZLayer_DEFAULT =  0.0f;
const f32 ZLayer_FOG     =  0.2f;
const f32 ZLayer_UI      =  0.9f;
const f32 ZLayer_CONSOLE =  1.0f;

enum RenderQueue {
    RenderQueue_OPAQUE,
    RenderQueue_TRANSPARENT,
};

struct DrawCallData {
    Vector4   override_color = V4_ONE;
    Vector2   uv_offset      = V2_ZERO;
    Vector2   uv_scale       = V2_ONE;
    Matrix4x4 transform;    // @Todo: this might be wastefull and slow.
};

struct DrawCallBuffer {
    u32 vao;
    u32 instance_buffer;
    i32 first_transparent;
    StaticArray<DrawCallData> data;
};

struct LineDrawCallData {
    Vector3 verticies[6];
};

struct LineDrawCallBuffer {
    u32 vao;
    u32 vbo;
    u32 vco;
    u32 data_length;
    LineDrawCallData data[1024];
};

struct RenderState {
    Program *current_shader;
    DrawCallBuffer draw_calls[TextureResource_COUNT];
    DrawCallBuffer font_draw_calls[FontResource_COUNT];
    LineDrawCallBuffer line_draw_calls;
};

internal void allocate_instance_buffer(Mesh *quad, u32 *vao, u32 *instance_buffer, u64 size) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    {   // Index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->ebo);
    }

    {   // Vertex data
        glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    {   // UV data
        glBindBuffer(GL_ARRAY_BUFFER, quad->tbo);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    // Instance data
    {
        glGenBuffers(1, instance_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, *instance_buffer);

        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size * sizeof(DrawCallData), nullptr, GL_STATIC_DRAW);

        i32 vector4_size = sizeof(Vector4);
        i32 stride = sizeof(DrawCallData);
        i32 attr_index = 2;
        u64 offset = 0;

        // OVERRIDE COLOR
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 4, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += vector4_size;
        attr_index += 1;

        // UV OFFSET
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 2, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += sizeof(Vector2);
        attr_index += 1;

        // UV SCALE
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 2, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += sizeof(Vector2);
        attr_index += 1;

        // MATRIX 0
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 4, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += vector4_size;
        attr_index += 1;

        // MATRIX 1
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 4, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += vector4_size;
        attr_index += 1;

        // MATRIX 2
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 4, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += vector4_size;
        attr_index += 1;

        // MATRIX 3
        glEnableVertexAttribArray(attr_index);
        glVertexAttribPointer(attr_index, 4, GL_FLOAT, GL_FALSE, stride, (void *)offset);
        glVertexAttribDivisor(attr_index, 1);
        offset += vector4_size;
        attr_index += 1;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

internal void allocate_line_instance_buffer(LineDrawCallBuffer &buffer) {
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    // VERTEX
    {
        glGenVertexArrays(1, &buffer.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

        u32 buffer_size = STATIC_ARRAY_CAPACITY(buffer.data) * sizeof(LineDrawCallData);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer.data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

#endif
