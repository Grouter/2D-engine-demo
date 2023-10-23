internal void handle_key_down(u8 scan_code, u16 virtual_code, bool alt_down) {
    if (input_state->keyboard_input_blocked) return;

    KeyInput key = {};
    key.scan_code    = scan_code;
    key.virtual_code = virtual_code;
    key.alt_down     = alt_down;
    key.ctrl_down    = (bool16)input_state->ctrl_down;
    key.pressed      = true;

    if (virtual_code == VK_CONTROL) {
        input_state->ctrl_down = true;
        key.ctrl_down = true;
    }
    else if (virtual_code == VK_ESCAPE) {
        platform_quit_program();
        return;
    }
#if defined(DEVELOPER)
    else if (virtual_code == VK_F1) {
        toggle_console();
    }
    else if (virtual_code == VK_F2) {
        debug_state->draw_tile_states = !debug_state->draw_tile_states;
    }
    else if (virtual_code == VK_F5) {
        toggle_editor();
    }

    if (game_state->console.open) return;

    if (program_state == ProgramState_EDITOR) {
        editor_handle_key(&key);
    }
#endif

    if (program_state == ProgramState_GAMEPLAY) {
        game_handle_key_down(&key);
    }
    else if (program_state == ProgramState_PROTOTYPE_INTRO) {
        if (virtual_code == VK_SPACE) {
            program_state = ProgramState_GAMEPLAY;
        }
    }
}

internal void handle_key_up(u8 scan_code, u16 virtual_code, bool alt_down) {
    if (input_state->keyboard_input_blocked) return;

    KeyInput key = {};
    key.scan_code    = scan_code;
    key.virtual_code = virtual_code;
    key.alt_down     = alt_down;
    key.ctrl_down    = (bool16)input_state->ctrl_down;
    key.pressed      = false;

    if (virtual_code == VK_CONTROL) {
        input_state->ctrl_down = false;
        key.ctrl_down = false;
    }
}

internal void handle_char(wchar_t c) {
    if (input_state->keyboard_input_blocked) return;

    KeyInput key = {};
    key.character = c;
    key.pressed   = true;

#if defined(DEVELOPER)
    if (game_state->console.open) {
        console_handle_char_input(&key);
    }
    else if (program_state == ProgramState_EDITOR) {
        editor_handle_char_input(&key);
    }
#endif
}

internal void handle_mouse_input() {
    input_state->mouse_delta.x = input_state->mouse.x - input_state->mouse_old.x;
    input_state->mouse_delta.y = input_state->mouse.y - input_state->mouse_old.y;

    input_state->mouse_old.x = input_state->mouse.x;
    input_state->mouse_old.y = input_state->mouse.y;

    input_state->mouse_viewport.x = max(min(input_state->mouse.x, game_state->viewport.width ) - game_state->viewport.left, 0);
    input_state->mouse_viewport.y = max(min(input_state->mouse.y, game_state->viewport.height) - game_state->viewport.bottom, 0);

    // Mouse unit position
    {
        input_state->mouse_unit.x = (f32)input_state->mouse_viewport.x;
        input_state->mouse_unit.y = (f32)game_state->viewport.height - (f32)input_state->mouse_viewport.y;

        input_state->mouse_unit *= game_state->pixels_to_units;
    }

    // Calculate mouse grid pos
    {
        i32 grid_tile_pixel_size = (i32)(game_state->unit_tile_size * game_state->unit_to_pixels);

        i32 offset_x = (i32)(game_state->tile_area_offset.x * game_state->unit_to_pixels);
        i32 offset_y = (i32)(game_state->tile_area_offset.y * game_state->unit_to_pixels);

        input_state->mouse_grid.x = (i32)(input_state->mouse_viewport.x - offset_x) / grid_tile_pixel_size;
        input_state->mouse_grid.y = (i32)(game_state->viewport.height - input_state->mouse_viewport.y - offset_y) / grid_tile_pixel_size;
    }
}

internal void handle_mouse_left_click() {
    if (input_state->mouse_input_blocked) return;

    if (program_state == ProgramState_GAMEPLAY) {
        game_handle_left_mouse_button();
    }
    else if (program_state == ProgramState_EDITOR) {
        editor_handle_left_mouse_button();
    }
}

internal void handle_mouse_right_click() {
    if (input_state->mouse_input_blocked) return;

    if (program_state == ProgramState_GAMEPLAY) {
        game_handle_right_mouse_button();
    }
    else if (program_state == ProgramState_EDITOR) {
        editor_handle_right_mouse_button();
    }
}
