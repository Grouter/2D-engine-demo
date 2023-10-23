internal void command_ping(CommandToken *args, u32 arg_count) {
    console_add_to_history("Pong!");
}

internal void command_add(CommandToken *args, u32 arg_count) {
    i32 a, b;
    command_parse_args2(args, &a, &b);

    console_add_to_history("%d + %d = %d", a, b, a + b);
}

internal void command_load(CommandToken *args, u32 arg_count) {
    assert(args[1].length < 32);

    char level_to_load[32];
    strncpy_s(level_to_load, args[1].start, args[1].length);

    console_add_to_history("Loading level: %s", level_to_load);

    HighResClock clock = start_clock();
    bool success = load_level(level_to_load);
    i64 time = stop_clock(&clock);

    if (success) {
        console_add_to_history("Loaded! (%ld micros)", time);
    }
    else console_add_to_history("Error!");
}

internal void command_save(CommandToken *args, u32 arg_count) {
    if (program_state != ProgramState_EDITOR) {
        console_add_to_history("You need to be in EDITOR mode!");
        return;
    }

    console_add_to_history("Saving %s...", game_state->current_level);

    bool success = save_level(game_state->current_level, game_state);

    if (success) console_add_to_history("Saved!");
    else console_add_to_history("Save failed.");
}

internal void command_new_level(CommandToken *args, u32 arg_count) {
    if (program_state != ProgramState_EDITOR) {
        console_add_to_history("You need to be in EDITOR mode!");
        return;
    }

    assert(args[1].length < 32);

    char new_level_name[32];
    strncpy_s(new_level_name, args[1].start, args[1].length);

    console_add_to_history("Creating new level %s...", new_level_name);

    bool success = new_level(new_level_name);

    if (success) console_add_to_history("New level created!");
    else console_add_to_history("Error creating a new file!");
}

internal void command_clear(CommandToken *args, u32 arg_count) {
    clear_console();
}
