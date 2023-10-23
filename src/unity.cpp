#if defined(DEVELOPER)

#else
    #define NDEBUG
#endif

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM

#define GLEW_STATIC

#include <windows.h>
#include <Windowsx.h>
#include <glew/glew.h>
#include <glew/wglew.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <math.h>
#include <string>
#include <cmath>
#include <assert.h>
#include <algorithm>
#include <time.h>

#include <stb_rect_pack.h>
#include <stb_image.h>
#include <stb_truetype.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_impl_win32.cpp>
#include <imgui/imgui_impl_opengl3.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/imgui_draw.cpp>

#include "types.h"
#include "platform.h"
#include "utils.h"
#include "benchmark.h"

#define MAX_LEVEL_NAME_LEN 64

#define VIRTUAL_WINDOW_W 32
#define VIRTUAL_WINDOW_H 18

#define INITIAL_WINDOW_W 1920
#define INITIAL_WINDOW_H 1080

#define TARGET_ASPECT_W 16
#define TARGET_ASPECT_H 9

#include "memory.h"
#include "debug.h"
#include "array.h"
#include "bucket_array.h"
#include "math/math.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/shapes.h"
#include "color.h"
#include "catalog.h"
#include "string_builder.h"
#include "tokenizer.h"
#include "parse_utils.cpp"

#include "shared.h"

#include "input.h"
#include "graphics.h"
#include "font.h"
#include "resources.h"
#include "camera.h"
#include "render.h"
#include "text_input.h"
#include "entity.h"
#include "collision.h"
#include "transaction.h"
#include "console.h"
#include "editor.h"
#include "commands.h"
#include "level.h"
#include "game.h"

// @Todo: we could have all of these under GameState...
global Resources      *resources;
global GameState      *game_state;
global RenderState    *render_state;
global InputState     *input_state;
global DebugState     *debug_state;

#include "tokenizer.cpp"
#include "tiles.cpp"
#include "collision.cpp"
#include "graphics.cpp"
#include "font.cpp"
#include "resources.cpp"
#include "serialize.cpp"
// #include "hotload.cpp"
#include "camera.cpp"
#include "render.cpp"
#include "text_input.cpp"
#include "transaction.cpp"
#include "entity.cpp"
#include "level.cpp"
#include "console.cpp"
#include "debug.cpp"
#include "save.cpp"
#include "game.cpp"
#include "editor.cpp"
#include "input.cpp"
#include "commands.cpp"
#include "prototype_intro.cpp"
#include "prototype_end.cpp"

#include "win32.h"
