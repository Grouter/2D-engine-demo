#if !defined(COLOR_H)
#define COLOR_H

struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

const Color Color_WHITE = Color { 255, 255, 255, 255 };
const Color Color_BLACK = Color { 0, 0, 0, 255 };
const Color Color_RED   = Color { 255, 0, 0, 255 };
const Color Color_GREEN = Color { 0, 255, 0, 255 };
const Color Color_BLUE  = Color { 0, 0, 255, 255 };
const Color Color_PINK  = Color { 255, 0, 255, 255 };

inline Color make_color(u8 r, u8 g, u8 b, u8 a) {
    Color result = { r, g, b, a };

    return result;
}

inline Color make_color(u8 r, u8 g, u8 b) {
    Color result = { r, g, b, 255 };

    return result;
}

inline Color make_color(u8 v) {
    Color result = { v, v, v, 255 };

    return result;
}

inline Vector4 color_to_v4(const Color &color) {
    Vector4 unit;

    unit.r = (f32)color.r / 255.0f;
    unit.g = (f32)color.g / 255.0f;
    unit.b = (f32)color.b / 255.0f;
    unit.a = (f32)color.a / 255.0f;

    return unit;
}

inline bool operator== (const Color &a, const Color &b) {
    return (
        a.r == b.r &&
        a.g == b.g &&
        a.b == b.b &&
        a.a == b.a
    );
}

inline bool operator!= (const Color &a, const Color &b) {
    return !(a == b);
}

#endif
