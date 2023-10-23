#ifdef VERTEX

// Vertex data
in vec3 position;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position, 1.0);
}

#endif

#ifdef FRAGMENT

uniform vec4 material_color;

out vec4 fragment_color;

void main() {
    fragment_color = vec4(1, 0, 1, 1);
}

#endif