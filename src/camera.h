#if !defined(CAMERA_H)
#define CAMERA_H

struct Camera {
    f32 clip_near;
    f32 clip_far;

    Matrix4x4 perspective;
    Matrix4x4 transform;
};

#endif
