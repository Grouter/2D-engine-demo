#if !defined(RESOURCES_H)
#define RESOURCES_H

enum ShaderResource {
    ShaderResource_DEFAULT,
    ShaderResource_LINE,
    ShaderResource_FOV,

    ShaderResource_COUNT
};

enum MeshResource {
    MeshResource_QUAD,
};

enum TextureResource {
    TextureResource_WHITE,
    TextureResource_MAIN,

    TextureResource_COUNT
};

// Adding or removing fonts will probably break the renderer!!
// There is no reason now to expect to have more than 3 font sizes loaded...
enum FontResource {
    FontResource_SMALL,
    FontResource_MEDIUM,
    FontResource_BIG,

    FontResource_COUNT
};

struct Resources {
    Mesh    meshes[1];
    Program programs[ShaderResource_COUNT];
    Texture textures[TextureResource_COUNT];
    Font    fonts[FontResource_COUNT];
};

#endif
