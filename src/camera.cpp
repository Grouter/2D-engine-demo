#include "platform.h"
#include "camera.h"

internal Camera create_camera(u32 window_w, u32 window_h) {
    Camera camera = {};

    camera.clip_near = -1.0f;
    camera.clip_far  =  1.0f;

    camera.perspective = ortho(-1.0f, 1.0f, (f32)window_w, (f32)window_h);

    return camera;
}

inline Vector3 get_forward_vector(Camera &camera) {
    return { camera.transform.raw[2], camera.transform.raw[6], camera.transform.raw[10] };
}

inline Vector3 get_up_vector(Camera &camera) {
    return { camera.transform.raw[1], camera.transform.raw[5], camera.transform.raw[9] };
}

inline Vector3 get_side_vector(Camera &camera) {
    return { camera.transform.raw[0], camera.transform.raw[4], camera.transform.raw[8] };
}
