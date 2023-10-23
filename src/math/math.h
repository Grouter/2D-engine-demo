#if !defined(MATH_H)
#define MATH_H

#define PI              3.14159265358979f
#define TWO_PI          (PI * 2.0f)
#define HALF_PI         (PI * 0.5f)
#define ONE_AND_HALF_PI (PI * 1.5f)

#define TO_RADIANS(v) ((v) * (PI / 180.0f))
#define TO_DEGREES(v) ((v) * (180.0f / PI))

#define CLAMP(val, min_val, max_val) (max(min_val, min(max_val, val)))

internal f32 lerp(f32 a, f32 b, f32 t) {
    return (a + ((b - a) * t));
}

internal i32 sign(i32 a) {
    if (a > 0) return  1;
    if (a < 0) return -1;
    return  0;
}

#endif
