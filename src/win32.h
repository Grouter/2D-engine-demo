#if !defined(WIN_32_H)
#define WIN_32_H

#define WIN32_CLASS_NAME "WindowClass"

internal HGLRC win32_create_gl_context(HDC window_context) {
    const i32 pixel_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_ALPHA_BITS_ARB,     8,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB,        4,
        0
    };

    i32 pixel_format_ID;
    u32 num_formats;

    bool status = wglChoosePixelFormatARB(window_context, pixel_attribs, 0, 1, &pixel_format_ID, &num_formats);
    if (!status || num_formats == 0) {
        log_print("GL could not choose a pixel format!\n");
        exit(1);
    }

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(window_context, pixel_format_ID, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(window_context, pixel_format_ID, &pfd);

    i32 context_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    HGLRC gl_render_context = wglCreateContextAttribsARB(window_context, NULL, context_attribs);
    if (gl_render_context == NULL) {
        log_print("OpenGL render context could not be created!\n");
        exit(1);
    }

    if (!wglMakeCurrent(window_context, gl_render_context)) {
        log_print("Could not set opengl render context to window context\n");
        exit(1);
    }

    // V-Sync
    wglSwapIntervalEXT(1);

    // Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Depth tests
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Alhpa blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    OutputDebugStringA("OpenGL initialized:\n");

    OutputDebugStringA("    Vendor: ");
    OutputDebugStringA((char*)glGetString(GL_VENDOR));
    OutputDebugStringA("\n");

    OutputDebugStringA("    Renderer: ");
    OutputDebugStringA((char*)glGetString(GL_RENDERER));
    OutputDebugStringA("\n");

    OutputDebugStringA("    Version: ");
    OutputDebugStringA((char*)glGetString(GL_VERSION));
    OutputDebugStringA("\n");

    return gl_render_context;
}

LRESULT CALLBACK win32_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 1;

#if defined(DEVELOPER)
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
        return 1;
    }
#endif

    switch (message) {
        case WM_CREATE: {
            // Fetch initial window dimensions
            RECT window_rect;
            if (GetWindowRect(window, &window_rect)) {
                on_window_created(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
            }
        } break;

        case WM_SIZE: {
            u32 width  = LOWORD(l_param);
            u32 height = HIWORD(l_param);

            on_window_resize(width, height);
        } break;

        case WM_DESTROY: {
            PostQuitMessage(0);

            running = false;
        } break;

        case WM_KEYUP:
        case WM_KEYDOWN: {
            bool is_up = message == WM_KEYUP;

            u16 virtual_code = LOWORD(w_param);
            u8 scan_code     = LOBYTE(HIWORD(l_param));
            bool alt_down    = (HIWORD(l_param) & KF_ALTDOWN) == KF_ALTDOWN;

            if (is_up) {
                handle_key_up(scan_code, virtual_code, alt_down);
            }
            else {
                bool repeat_flag = (HIWORD(l_param) & KF_REPEAT) == KF_REPEAT;
                if (repeat_flag) break; // We will handle repeat our way...

                handle_key_down(scan_code, virtual_code, alt_down);
            }
        } break;

        case WM_CHAR: {
            wchar_t pressed_char = (wchar_t)w_param;

            handle_char(pressed_char);
        } break;

        case WM_MOUSEMOVE: {
            input_state->mouse.x = GET_X_LPARAM(l_param);
            input_state->mouse.y = GET_Y_LPARAM(l_param);
        } break;

        case WM_LBUTTONDOWN: {
            handle_mouse_left_click();
        } break;

        case WM_RBUTTONDOWN: {
            handle_mouse_right_click();
        } break;

        default: {
            result = DefWindowProc(window, message, w_param, l_param);
        }
    }

    return result;
}

internal void win32_create_window(HINSTANCE instance, WNDPROC callback, int show_code, HDC *window_context, HGLRC *opengl_context, HWND *window_handle) {
    WNDCLASS window_class = {};

    window_class.lpfnWndProc   = callback;
    window_class.hInstance     = instance;
    window_class.lpszClassName = TEXT(WIN32_CLASS_NAME);
    //window_class.hIcon    // @Todo: add icon later

    RegisterClass(&window_class);

    RECT w_rect;
    w_rect.left   = 100;
    w_rect.right  = 100 + INITIAL_WINDOW_W;
    w_rect.bottom = 100 + INITIAL_WINDOW_H;
    w_rect.top    = 100;

    AdjustWindowRect(&w_rect, WS_OVERLAPPEDWINDOW, false);

    *window_handle = CreateWindowEx(
        0,
        window_class.lpszClassName,
        TEXT("Game1"),
        WS_OVERLAPPEDWINDOW,
        w_rect.left, w_rect.top,
        w_rect.right - w_rect.left, w_rect.bottom - w_rect.top,
        NULL, NULL,
        instance,
        NULL
    );

    if (*window_handle == NULL) {
        log_print("Window failed to create!\n");
        exit(1);
    }

    *window_context = GetDC(*window_handle);
    *opengl_context = win32_create_gl_context(*window_context);

    ShowWindow(*window_handle, show_code);
    UpdateWindow(*window_handle);
}


internal void platform_print_to_console(const char *to_print) {
    OutputDebugStringA(to_print);
}

internal void platform_quit_program() {
    HWND window_handle = GetActiveWindow();
    PostMessage(window_handle, WM_CLOSE, 0, 0);
}

// To initialzie GL extensions, we need to create a fake window
// which will be destroyed after extensions are initialzied...
internal void win32_init_gl_extensions() {
    WNDCLASS window_class = {};

    window_class.lpfnWndProc   = DefWindowProcA;
    window_class.hInstance     = GetModuleHandle(0);
    window_class.lpszClassName = TEXT("FakeWindowClass");

    RegisterClass(&window_class);

    HWND fake_window = CreateWindowEx(
        0,
        window_class.lpszClassName,
        TEXT("FakeWindow"),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 1, 1,
        NULL, NULL,
        window_class.hInstance,
        NULL
    );

    HDC fake_DC = GetDC(fake_window);
    if (!fake_DC) {
        log_print("Create a fake window!\n");
        exit(1);
    }

    PIXELFORMATDESCRIPTOR fake_PFD = {};
    fake_PFD.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    fake_PFD.nVersion    = 1;
    fake_PFD.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    fake_PFD.dwLayerMask = PFD_MAIN_PLANE;
    fake_PFD.cColorBits  = 32;
    fake_PFD.cAlphaBits  = 8;
    fake_PFD.cDepthBits  = 16;

    int fake_PFD_ID = ChoosePixelFormat(fake_DC, &fake_PFD);
    if (!fake_PFD_ID) {
        log_print("Could not choose a fake pixel format!\n");
        exit(1);
    }

    if (!SetPixelFormat(fake_DC, fake_PFD_ID, &fake_PFD)) {
        log_print("Could not set a fake pixel format!\n");
        exit(1);
    }

    HGLRC fake_RC = wglCreateContext(fake_DC);
    if (!fake_RC) {
        log_print("Could create a fake gl render context!\n");
        exit(1);
    }

    if (!wglMakeCurrent(fake_DC, fake_RC)) {
        log_print("Could not set a fake render context!\n");
        exit(1);
    }

    GLenum glew_state = glewInit();
    if (glew_state != GLEW_OK) {
        log_print("Error while initializing GL extensions!\n");
        exit(1);
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(fake_RC);
    ReleaseDC(fake_window, fake_DC);
    DestroyWindow(fake_window);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int show_code) {
    HDC   window_context;
    HGLRC opengl_context;
    HWND  window_handle;

    // Allocate global game memory
    {
        game_memory = {};
        game_memory.bytes = megabytes(4);
        game_memory.base = (u8 *)VirtualAlloc(0, game_memory.bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        assert(game_memory.base);
        log_print("Allocated %u bytes of global game memory!\n", game_memory.bytes);
    }

    // Assign game's state structs
    {
        resources = push_struct(&game_memory, Resources);
        *resources = {};

        game_state = push_struct(&game_memory, GameState);
        *game_state = {};

        render_state = push_struct(&game_memory, RenderState);
        *render_state = {};

        input_state = push_struct(&game_memory, InputState);
        *input_state = {};

        debug_state = push_struct(&game_memory, DebugState);
        *debug_state = {};

        log_print("Assigned %u bytes of static game memory!\n", game_memory.used);
    }

    // Assign game's permanent memory
    {
        MemoryArena *permanent_memory = &game_state->permanent_memory;
        *permanent_memory = {};
        permanent_memory->bytes = megabytes(1);
        permanent_memory->base = (u8 *)_push_to_memory_arena(&game_memory, permanent_memory->bytes);

        log_print("Assigned %u bytes of permanent memory!\n", permanent_memory->bytes);
    }

    // Assign game's temporary memory
    {
        TemporaryMemoryArena *temporary_memory = &game_state->temporary_memory;
        *temporary_memory = {};
        temporary_memory->bytes = megabytes(1);
        temporary_memory->base = (u8 *)_push_to_memory_arena(&game_memory, temporary_memory->bytes);

        log_print("Assigned %u bytes of temporary memory!\n", temporary_memory->bytes);
    }

    log_print("Remaining global memory: %u\n", game_memory.bytes - game_memory.used);

    win32_init_gl_extensions();

    win32_create_window(instance, win32_window_callback, show_code, &window_context, &opengl_context, &window_handle);

#if defined(DEVELOPER)
    // Dear ImGUI stuff
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO(); (void)io;
        io.WantCaptureMouse = true;
        io.Fonts->AddFontFromFileTTF("Merriweather-Regular.ttf", 18.0f);

        // Win32
        ImGui_ImplWin32_Init(window_handle);

        // OpenGL
        ImGui_ImplOpenGL3_Init(GLSL_VERSION);

        // BG color
        ImGui::StyleColorsClassic();
    }
#endif

    init_game();

    MSG message = {};

    u64 current_time = millis();

    while (running) {
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);

            continue;
        }

        game_state->temporary_memory.used = 0;  // Reset temporary memory

#if defined(DEVELOPER)
        {
            ImGuiIO &io = ImGui::GetIO(); (void)io;
            input_state->mouse_input_blocked = io.WantCaptureMouse;
            input_state->keyboard_input_blocked = io.WantCaptureKeyboard;
        }
#endif

        handle_mouse_input();

        u64 new_time = millis();
        u64 frame_time = new_time - current_time;
        current_time = new_time;

        f32 delta_time = (f32)frame_time / 1000.0f;

        game_state->time_elapsed += delta_time;

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if defined(DEVELOPER)
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        draw_console(delta_time);
        draw_debug_info(delta_time);
#endif

        switch (program_state) {
            case ProgramState_EDITOR : {
                draw_editor(delta_time);
                draw_entities_for_editor(&game_state->entities);
                draw_debug_window();

                if (debug_state->draw_tile_states) draw_debug_tile_states();
                if (debug_state->draw_tile_edges)  draw_debug_tile_edges();
            } break;

            case ProgramState_GAMEPLAY : {
                update_and_render_game(delta_time);
                draw_debug_window();

                if (debug_state->draw_tile_states) draw_debug_tile_states();
                if (debug_state->draw_tile_edges)  draw_debug_tile_edges();
            } break;

            case ProgramState_PROTOTYPE_INTRO : {
                draw_prototype_intro();
            } break;

            case ProgramState_PROTOTYPE_END : {
                draw_prototype_end();
            } break;
        }

        flush_draw_calls();

#if defined(DEVELOPER)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

        SwapBuffers(window_context);
    }

    cleanup_game();

#if defined(DEVELOPER)
    // ImGUI
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        log_print("ImGUI cleaned up!\n");
    }
#endif

    // Win32
    {
        if (opengl_context) {
            wglDeleteContext(opengl_context);
        }

        if (window_context) {
            ReleaseDC(window_handle, window_context);
        }

        DestroyWindow(window_handle);

        opengl_context = 0;
        window_context = 0;

        UnregisterClass(TEXT(WIN32_CLASS_NAME), instance);

        log_print("Win32 cleaned up!\n");
    }

    return 0;
}

#endif
