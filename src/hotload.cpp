global std::mutex _hotload_mutex;
global HANDLE _watch_handle;

struct HotloadShaderEntry {
    char shader_name[64];
};

global Array<HotloadShaderEntry> _hotload_shader_queue;

internal void init_hotload() {
    allocate_array(_hotload_shader_queue, 4);
}

internal void hotload_watcher() {
    log_print("Hotload started!\n");

    _watch_handle = CreateFile(
        "./",   // the asset folder
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr
    );

    u8 *buffer = (u8 *)calloc(2048, 1);
    char file_path[2048];

    DWORD b_returned;

    while (running) {
        bool result = ReadDirectoryChangesW(
            _watch_handle,
            buffer,
            2048,
            true,
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            &b_returned,
            nullptr,
            nullptr
        );

        if (result == 0) {
            log_print("Read dir changes failed err code: %d\n", GetLastError());
            break;
        }

        FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)buffer;

        if (info->Action != FILE_ACTION_MODIFIED) continue;

        u64 file_path_size = info->FileNameLength / sizeof(wchar_t);

        // Reset the filename buffer and convert wide chars to normal chars
        // while writing the the filename buffer
        memset(file_path, 0, 2048);
        for (u64 i = 0; i < file_path_size; i++) {
            file_path[i] = (char)info->FileName[i];
        }

        log_print("Hotload trigger: %s\n", file_path);

        char *file_name = file_path + file_path_size;
        while (file_name != file_path) {
            if (*file_name == '\\' || *file_name == '/') {
                file_name += 1;
                break;
            }
            file_name -= 1;
        }

        u64 file_name_size = file_path_size - (file_name - file_path);

        // File extension
        char *extension = file_path;
        extension = eat_until(extension, '.');
        extension += 1;

        if (strncmp(extension, "glsl", 4) == 0) {
            // @Todo: I don't like this for loop check
            bool already_in_queue = false;
            for (u64 i = 0; i < _hotload_shader_queue.length; i++) {
                char *name = _hotload_shader_queue[i].shader_name;

                if (strncmp(name, file_name, file_name_size) == 0) {
                    already_in_queue = true;
                    break;
                }
            }

            if (!already_in_queue) {
                _hotload_mutex.lock();

                HotloadShaderEntry shader_queue_entry = {};
                strncpy(shader_queue_entry.shader_name, file_name, file_name_size);
                _hotload_shader_queue.add(shader_queue_entry);

                _hotload_mutex.unlock();
            }
        }
    }

    free(buffer);
}

internal void process_hotload_queue(Resources &res) {
    _hotload_mutex.lock();

    for (i64 i = _hotload_shader_queue.length - 1; i >= 0; i--) {
        char *shader_name = _hotload_shader_queue[i].shader_name;

        char path[64];
        snprintf(path, 64, "shaders/%s", shader_name);

        u64 shader_hash = hash_string(path);

        for (u32 j = 0; j < ShaderResource_COUNT; j++) {
            Program &program = res.programs[j];

            if (program.file_hash == shader_hash) {
                log_print("Reloding shader: %s (old handle: %u)\n", shader_name, program.handle);

                u32 new_handle = 0;
                bool success = load_program(path, new_handle, (u32)program.shader_inputs.length, program.shader_inputs.data);

                if (success) {
                    glUseProgram(0);

                    glDeleteProgram(program.handle);
                    program.handle = new_handle;
                }
            }
        }

        _hotload_shader_queue.remove_last();
    }

    _hotload_mutex.unlock();
}

internal void hotload_terminate_file_watch() {
    CancelIoEx(_watch_handle, nullptr);
    CloseHandle(_watch_handle);
}
