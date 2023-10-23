#if !defined(COLLISION_H)
#define COLLISION_H

enum IntersectType {
    Intersect_NONE,
    Intersect_ENTITY,
    Intersect_BELT
};

struct RaycastHit {
    IntersectType type = Intersect_NONE;

    union HitPointer {
        Entity *entity;
    };

    Vector2i position;
    HitPointer hit;
};

struct HitData {
    u32 side;
    f32 hit_distance;
    Vector2i logical_position;
};

internal bool line_intersects_point(Vector2i line_a, Vector2i line_b, Vector2i point) {
    return (
        (   // Vertical
            (line_a.x == line_b.x)
            &&
            (point.x == line_a.x)
            &&
            (point.y >= min(line_a.y, line_b.y) && point.y <= max(line_a.y, line_b.y))
        )
        ||
        (   // Horizontal
            (line_a.y == line_b.y)
            &&
            (point.y == line_a.y)
            &&
            (point.x >= min(line_a.x, line_b.x) && point.x <= max(line_a.x, line_b.x))
        )
    );
}

internal bool line_segment_intersection(Edge line_a, Edge line_b, Vector2 *intersection) {
    f32 d = (line_a.a.x - line_a.b.x) * (line_b.a.y - line_b.b.y) - (line_a.a.y - line_a.b.y) * (line_b.a.x - line_b.b.x);

    if (d == 0) return false;

    f32 t_nom = (line_a.a.x - line_b.a.x) * (line_b.a.y - line_b.b.y) - (line_a.a.y - line_b.a.y) * (line_b.a.x - line_b.b.x);
    f32 u_nom = (line_a.a.x - line_b.a.x) * (line_a.a.y - line_a.b.y) - (line_a.a.y - line_b.a.y) * (line_a.a.x - line_a.b.x);

    f32 t = t_nom / d;
    f32 u = u_nom / d;

    if (0.0f <= t && t <= 1.0f && 0.0f <= u && u <= 1.0f) {
        intersection->x = line_a.a.x + t * (line_a.b.x - line_a.a.x);
        intersection->y = line_a.a.y + t * (line_a.b.y - line_a.a.y);

        return true;
    }

    return false;
}

internal bool line_intersects_ray(Vector2i a_a, Vector2i a_b, Vector2i ray_start, Vector2i ray_dir, i32 length, Vector2i &result) {
    ray_dir = ray_dir * length;
    Vector2i ray_b = ray_start + ray_dir;

    i32 t = (a_a.x - ray_start.x) * (ray_start.y - ray_b.y) - (a_a.y - ray_start.y) * (ray_start.x - ray_b.x);
    i32 u = (a_a.x - ray_start.x) * (a_a.y - a_b.y) - (a_a.y - ray_start.y) * (a_a.x - a_b.x);

    i32 d = (a_a.x - a_b.x) * (ray_start.y - ray_b.y) - (a_a.y - a_b.y) * (ray_start.x - ray_b.x);

    if (d == 0) {
        Vector2i a_delta = a_b - a_a;

        if (abs(a_delta.x) != abs(ray_dir.x) && abs(a_delta.y) != abs(ray_dir.y)) return false;

        if (a_a.x == ray_start.x || a_a.y == ray_start.y) {
            result = closest(ray_start, a_a, a_b);
            return true;
        }
        else return false;
    }
    else {
        bool intersect = (
            (d > 0 && t >= 0 && t <= d && u >= 0 && u <= d)
            ||
            (d < 0 && t <= 0 && t >= d && u <= 0 && u >= d)
        );

        if (intersect) {
            f32 u_d = (f32)u / d;
            result.x = ray_start.x + (i32)(u_d * (ray_b.x - ray_start.x));
            result.y = ray_start.y + (i32)(u_d * (ray_b.y - ray_start.y));

            return true;
        }
        else return false;
    }
}

#endif
