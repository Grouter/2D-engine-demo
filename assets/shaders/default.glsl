#ifdef VERTEX

// Vertex data
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

// Instance data
layout (location = 2) in vec4 override_color;
layout (location = 3) in vec2 uv_offset;
layout (location = 4) in vec2 uv_scale;
layout (location = 5) in mat4 model;

uniform mat4 projection;

out vec4 f_override_color;
out vec2 f_uv;
out vec2 f_uv_offset;
out vec2 f_uv_scale;

void main() {
    f_override_color = override_color;
    f_uv = uv;
    f_uv_offset = uv_offset;
    f_uv_scale = uv_scale;

    gl_Position = projection * model * vec4(position, 1.0);
}

#endif

#ifdef FRAGMENT

in vec4 f_override_color;
in vec2 f_uv;
in vec2 f_uv_offset;
in vec2 f_uv_scale;

uniform vec4 material_color;

uniform sampler2D diffuse_texture;
uniform bool diffuse_alpha_mask;

out vec4 fragment_color;

void main() {
    // Recalculates the new UVs according to the uv_data
    vec2 mapped_uv;
    mapped_uv.x = f_uv_offset.x + (f_uv.x * f_uv_scale.x);
    mapped_uv.y = f_uv_offset.y + (f_uv.y * f_uv_scale.y);

    vec4 texture_sample = texture(diffuse_texture, mapped_uv);

    if (diffuse_alpha_mask) {
        fragment_color = material_color;
        fragment_color.a *= texture_sample.r;
    }
    else {
        fragment_color = material_color * texture_sample;
    }

    fragment_color *= f_override_color;

    if (fragment_color.a == 0) discard;
}

#endif