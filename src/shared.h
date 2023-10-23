#if !defined(SHARED_H)
#define SHARED_H

enum ProgramState {
    ProgramState_GAMEPLAY,
    ProgramState_EDITOR,
    ProgramState_PROTOTYPE_INTRO,
    ProgramState_PROTOTYPE_END,
};

//
// Globals
//
global MemoryArena game_memory;

// Engine
#if defined(DEVELOPER)
global ProgramState program_state = ProgramState_GAMEPLAY;
#else
global ProgramState program_state = ProgramState_PROTOTYPE_INTRO;
#endif

global bool running = true;

//
// Proc
//
internal void init_game();
internal void update_and_render_game();
internal void cleanup_game();

// Libraries

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

#endif
