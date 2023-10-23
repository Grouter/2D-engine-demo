#if !defined(MATRIX_H)
#define MATRIX_H

union Matrix4x4 {
    f32 table[4][4];
    f32 raw[16];
};

inline Matrix4x4 identity() {
    Matrix4x4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    return result;
}

inline Matrix4x4 scaling(f32 x, f32 y, f32 z) {
    Matrix4x4 result = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };

    return result;
}

inline Matrix4x4 translation(f32 x, f32 y, f32 z) {
    Matrix4x4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    };

    return result;
}

inline Matrix4x4 rotation_x(f32 x) {
    f32 c_x = cos(x);
    f32 s_x = sin(x);

    Matrix4x4 result = {
        1,    0,   0, 0,
        0,  c_x, s_x, 0,
        0, -s_x, c_x, 0,
        0,    0,   0, 1
    };

    return result;
}

inline Matrix4x4 rotation_y(f32 y) {
    f32 c_y = cos(y);
    f32 s_y = sin(y);

    Matrix4x4 result = {
        c_y,  0, -s_y, 0,
          0,  1,    0, 0,
        s_y,  0,  c_y, 0,
          0,  0,    0, 1
    };

    return result;
}

inline Matrix4x4 rotation_z(f32 z) {
    f32 c_z = cos(z);
    f32 s_z = sin(z);

    Matrix4x4 result = {
        c_z, -s_z, 0, 0,
        s_z,  c_z, 0, 0,
          0,    0, 1, 0,
          0,    0, 0, 1
    };

    return result;
}

inline Matrix4x4 perspective(f32 fov, f32 clip_near, f32 clip_far, f32 display_w, f32 display_h) {
    f32 aspect = display_w / display_h;
    f32 fov_radians = TO_RADIANS(fov);
    f32 cotan_half_fov = 1.0f / tan(fov_radians / 2.0f);

    Matrix4x4 perspective = {};

    perspective.table[0][0] = cotan_half_fov / aspect;
    perspective.table[0][1] = 0.0f;
    perspective.table[0][2] = 0.0f;
    perspective.table[0][3] = 0.0f;

    perspective.table[1][0] = 0.0f;
    perspective.table[1][1] = cotan_half_fov;
    perspective.table[1][2] = 0.0f;
    perspective.table[1][3] = 0.0f;

    perspective.table[2][0] = 0.0f;
    perspective.table[2][1] = 0.0f;
    perspective.table[2][2] = (clip_far + clip_near) / (clip_near - clip_far);
    perspective.table[2][3] = -1.0f;

    perspective.table[3][0] =  0.0f;
    perspective.table[3][1] =  0.0f;
    perspective.table[3][2] = (2.0f * clip_far * clip_near) / (clip_near - clip_far);
    perspective.table[3][3] =  0.0f;

    return perspective;
}

inline Matrix4x4 ortho(f32 clip_near, f32 clip_far, f32 display_w, f32 display_h) {
    f32 left   = 0.0f;
    f32 right  = display_w;
    f32 top    = display_h;
    f32 bottom = 0.0f;

    Matrix4x4 orto = {};

    orto.table[0][0] = 2.0f / (right - left);
    orto.table[0][1] = 0.0f;
    orto.table[0][2] = 0.0f;
    orto.table[0][3] = 0.0f;

    orto.table[1][0] = 0.0f;
    orto.table[1][1] = 2.0f / (top - bottom);
    orto.table[1][2] = 0.0f;
    orto.table[1][3] = 0.0f;

    orto.table[2][0] = 0.0f;
    orto.table[2][1] = 0.0f;
    orto.table[2][2] = -2.0f / (clip_far - clip_near);
    orto.table[2][3] = 0.0f;

    orto.table[3][0] = -(right + left) / (right - left);
    orto.table[3][1] = -(top + bottom) / (top - bottom);
    orto.table[3][2] = -(clip_far + clip_near) / (clip_far - clip_near);
    orto.table[3][3] = 1.0f;

    return orto;
}

// Multiplies matricies (in column major order), that means it will swap them and will do "bXa"
internal Matrix4x4 multiply(const Matrix4x4 &a, const Matrix4x4 &b) {
    Matrix4x4 result = identity();

    // This is not supposed to be effective!
    for (int i = 0; i < 4; i++) {
        result.table[i][0] = a.table[0][0] * b.table[i][0] + a.table[1][0] * b.table[i][1] + a.table[2][0] * b.table[i][2] + a.table[3][0] * b.table[i][3];
        result.table[i][1] = a.table[0][1] * b.table[i][0] + a.table[1][1] * b.table[i][1] + a.table[2][1] * b.table[i][2] + a.table[3][1] * b.table[i][3];
        result.table[i][2] = a.table[0][2] * b.table[i][0] + a.table[1][2] * b.table[i][1] + a.table[2][2] * b.table[i][2] + a.table[3][2] * b.table[i][3];
        result.table[i][3] = a.table[0][3] * b.table[i][0] + a.table[1][3] * b.table[i][1] + a.table[2][3] * b.table[i][2] + a.table[3][3] * b.table[i][3];
    }

    return result;
}

inline void translate(Matrix4x4 &matrix, f32 x, f32 y, f32 z) {
    matrix.table[3][0] += x;
    matrix.table[3][1] += y;
    matrix.table[3][2] += z;
}

inline void rotate(Matrix4x4 &matrix, f32 x, f32 y, f32 z) {
    Matrix4x4 x_rotation = rotation_x(x);
    Matrix4x4 y_rotation = rotation_y(y);
    Matrix4x4 z_rotation = rotation_z(z);

    matrix = multiply(matrix, x_rotation);
    matrix = multiply(matrix, y_rotation);
    matrix = multiply(matrix, z_rotation);
}

inline void scale(Matrix4x4 &matrix, f32 x, f32 y, f32 z) {
    Matrix4x4 scale_m = scaling(x, y, z);

    matrix = multiply(matrix, scale_m);
}

#endif
