internal void save_game_state(GameState *game) {
    FILE *save_file = fopen("save.mind", "wb");

    {
        u64 hash = 0;

        if (game->last_overworld_index >= 0) {
            hash = hash_string(GAME_LEVELS[game->last_overworld_index].name);
        }

        fwrite(&hash, sizeof(u64), 1, save_file);
    }

    fwrite(&game->last_player_rotation, sizeof(game->last_player_rotation), 1, save_file);
    fwrite(&game->overworld_last_position, sizeof(game->overworld_last_position), 1, save_file);

    {
        u64 level_state_count = (u64)STATIC_ARRAY_CAPACITY(GAME_LEVELS);

        fwrite(&level_state_count, sizeof(level_state_count), 1, save_file);

        for (u32 i = 0; i < level_state_count; i++) {
            fwrite(_GAME_LEVEL_HASHES + i, sizeof(_GAME_LEVEL_HASHES[0]), 1, save_file);
            fwrite(game->levels_solved_state + i, sizeof(game->levels_solved_state[0]), 1, save_file);
        }
    }

    fclose(save_file);
}

internal void load_game_state(GameState *game) {
    FILE *save_file = fopen("save.mind", "rb");

    if (save_file) {
        {
            u64 overworld_hash = 0;
            fread(&overworld_hash, sizeof(u64), 1, save_file);
            game->last_overworld_index = get_level_index(overworld_hash);
        }

        fread(&game->last_player_rotation, sizeof(game->last_player_rotation), 1, save_file);
        fread(&game->overworld_last_position, sizeof(game->overworld_last_position), 1, save_file);

        {
            u64 level_state_count = 0;

            fread(&level_state_count, sizeof(level_state_count), 1, save_file);

            for (u32 i = 0; i < level_state_count; i++) {
                u64 level_hash = 0;
                bool solved = false;

                fread(&level_hash, sizeof(u64), 1, save_file);
                fread(&solved, sizeof(bool), 1, save_file);

                i32 target_level_index = get_level_index(level_hash);

                if (target_level_index >= 0) {
                    game->levels_solved_state[target_level_index] = solved;
                }
            }
        }

        fclose(save_file);
    }
}
