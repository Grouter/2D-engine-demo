#if !defined(VECTOR_H)
#define VECTOR_H

union Vector2 {
    struct {
        f32 x;
        f32 y;
    };
    f32 data[2];
};

union Vector2i {
    struct {
        i32 x;
        i32 y;
    };
    i32 data[2];
};

union Vector3 {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    f32 data[3];
};

union Vector4 {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 data[4];
};

//
// :Vector2
//

const Vector2 V2_ZERO  = { 0.0f, 0.0f };
const Vector2 V2_ONE   = { 1.0f, 1.0f };
const Vector2 V2_UP    = { 0.0f, 1.0f };
const Vector2 V2_RIGHT = { 1.0f, 0.0f };

inline Vector2 make_vector2(f32 x, f32 y) {
    Vector2 result = { x, y };

    return result;
}

inline Vector2 make_vector2(f32 value) {
    Vector2 result = { value, value };

    return result;
}

inline void dump_vector(Vector2 a) {
    log_print("Vector2: %f %f\n", a.x, a.y);
}

inline f32 length(Vector2 a) {
    f32 result = sqrt(a.x * a.x + a.y * a.y);

    return result;
}

inline Vector2 normalized(Vector2 a) {
    f32 l = length(a);

    if (l == 0) return a;

    a.x /= l;
    a.y /= l;

    return a;
}

inline Vector2 operator* (Vector2 a, Vector2 b) {
    Vector2 result;

    result.x = a.x * b.x;
    result.y = a.y * b.y;

    return result;
}

inline Vector2 operator* (Vector2 a, f32 b) {
    Vector2 result;

    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

inline Vector2 operator*= (Vector2 &a, f32 b) {
    a.x *= b;
    a.y *= b;

    return a;
}

inline Vector2 operator+ (Vector2 a, Vector2 b) {
    Vector2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline Vector2 operator+= (Vector2 &a, Vector2 b) {
    a.x += b.x;
    a.y += b.y;

    return a;
}

inline Vector2 operator- (Vector2 a, Vector2 b) {
    Vector2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline Vector2 operator-= (Vector2 &a, Vector2 b) {
    a.x -= b.x;
    a.y -= b.y;

    return a;
}

inline Vector2 direction(Vector2 from, Vector2 to) {
    Vector2 result = to - from;

    result = normalized(result);

    return result;
}

inline f32 distance(Vector2 a, Vector2 b) {
    Vector2 line = b - a;

    f32 result = length(line);

    return result;
}

inline Vector2 lerp(Vector2 a, Vector2 b, f32 t) {
    Vector2 result;

    result.x = lerp(a.x, b.x, t);
    result.y = lerp(a.y, b.y, t);

    return result;
}

// Returns rotated original vector by radians anti-clockwise
inline Vector2 rotated(Vector2 original, f32 radians) {
    Vector2 result;

    f32 cos_d = cos(radians);
    f32 sin_d = sin(radians);

    result.x = cos_d * original.x - sin_d * original.y;
    result.y = sin_d * original.x + cos_d * original.y;

    return result;
}

inline f32 dot(Vector2 a, Vector2 b) {
    return (a.x * b.x) + (a.y * b.y);
}

inline f32 angle(Vector2 a, Vector2 b) {
    return atan2f(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
}

//
// :Vector2i
//

const Vector2i V2i_UP    = {  0,  1 };
const Vector2i V2i_LEFT  = { -1,  0 };
const Vector2i V2i_DOWN  = {  0, -1 };
const Vector2i V2i_RIGHT = {  1,  0 };

inline Vector2i make_vector2i(i32 x, i32 y) {
    Vector2i result = { x, y };

    return result;
}

inline Vector2i make_vector2i(i32 value) {
    Vector2i result = { value, value };

    return result;
}

inline void dump_vector(Vector2i v) {
    log_print("GridPosition: %d %d\n", v.x, v.y);
}

inline Vector2i operator* (Vector2i a, i32 b) {
    Vector2i result;

    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

inline Vector2i operator+ (Vector2i a, Vector2i b) {
    Vector2i result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline Vector2i operator+= (Vector2i &a, Vector2i b) {
    a.x += b.x;
    a.y += b.y;

    return a;
}


inline Vector2i operator- (Vector2i a, Vector2i b) {
    Vector2i result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

inline bool operator== (Vector2i a, Vector2i b) {
    return a.x == b.x && a.y == b.y;
}

inline Vector2i rotation_to_direction(f32 rotation) {
    Vector2i offset;

    offset.x = (i32)sin(rotation);
    offset.y = (i32)cos(rotation);

    return offset;
}

inline f32 length(Vector2i v) {
    return sqrtf((f32)v.x * v.x + v.y * v.y);
}

inline i32 length_no_sqrt(Vector2i v) {
    return v.x * v.x + v.y * v.y;
}

inline f32 distance(Vector2i a, Vector2i b) {
    Vector2i delta = a - b;

    return length(delta);
}

inline i32 dot(Vector2i a, Vector2i b) {
    return (a.x * b.x) + (a.y * b.y);
}

inline f32 angle(Vector2i a, Vector2i b) {
    f32 d = (f32)dot(a, b);
    f32 a_len = length(a);
    f32 b_len = length(b);

    return acos(d / (a_len * b_len));
}

// If both are the in the same distance, a is returned
inline Vector2i closest(Vector2i to, Vector2i a, Vector2i b) {
    i32 a_l = length_no_sqrt(a - to);
    i32 b_l = length_no_sqrt(b - to);

    if (a_l <= b_l) return a;
    else return b;
}

//
// :Vector3
//

const Vector3 V3_UP   = { 0.0f, 1.0f, 0.0f };
const Vector3 V3_ZERO = { 0.0f, 0.0f, 0.0f };

inline Vector3 make_vector3(f32 x, f32 y, f32 z) {
    Vector3 result = { x, y, z };

    return result;
}

inline Vector3 make_vector3(f32 value) {
    Vector3 result = { value, value, value };

    return result;
}

inline Vector3 make_vector3(Vector2 xy, f32 z) {
    Vector3 result = { xy.x, xy.y, z };

    return result;
}

inline void dump_vector(Vector3 a) {
    log_print("Vector3: %f %f %f\n", a.x, a.y, a.z);
}

inline f32 length(Vector3 a) {
    f32 result = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);

    return result;
}

inline Vector3 normalized(Vector3 a) {
    f32 l = length(a);

    if (l == 0) return a;

    a.x /= l;
    a.y /= l;
    a.z /= l;

    return a;
}

inline Vector3 cross(Vector3 a, Vector3 b) {
    Vector3 cross;

    cross.x = (a.y * b.z) - (a.z * b.y);
    cross.y = (a.z * b.x) - (a.x * b.z);
    cross.z = (a.x * b.y) - (a.y * b.x);

    return cross;
}

inline Vector3 operator* (Vector3 a, f32 b) {
    Vector3 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;

    return result;
}

inline Vector3 operator*= (Vector3 &a, f32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;

    return a;
}

inline Vector3 operator+ (Vector3 a, Vector3 b) {
    Vector3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline Vector3 operator+= (Vector3 &a, Vector3 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;

    return a;
}

inline Vector3 operator- (Vector3 a, Vector3 b) {
    Vector3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline Vector3 direction(Vector3 from, Vector3 to) {
    Vector3 result = to - from;

    result = normalized(result);

    return result;
}

inline f32 distance(Vector3 a, Vector3 b) {
    Vector3 line = b - a;

    f32 result = length(line);

    return result;
}

//
// :Vector4
//

const Vector4 V4_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };

inline Vector4 make_vector4(f32 x, f32 y, f32 z, f32 w) {
    Vector4 result = { x, y, z, w };

    return result;
}

inline void dump_vector(Vector4 a) {
    log_print("Vector4: %f %f %f %f\n", a.x, a.y, a.z, a.w);
}

#endif
