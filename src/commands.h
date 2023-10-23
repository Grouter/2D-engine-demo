#if !defined(COMMANDS_H)
#define COMMANDS_H

struct CommandToken {
    char *start;
    size_t length;
};

struct Command {
    char name[16];
    u32 name_size;
    u32 arg_count;
    void (*proc)(CommandToken *args, u32 arg_count);
};

// Forward declare commands here:
internal void command_ping(CommandToken *args, u32 arg_count);
internal void command_add(CommandToken *args, u32 arg_count);
internal void command_save(CommandToken *args, u32 arg_count);
internal void command_load(CommandToken *args, u32 arg_count);
internal void command_new_level(CommandToken *args, u32 arg_count);
internal void command_clear(CommandToken *args, u32 arg_count);

internal void _convert(const char *src, f32 *loc) { *loc = (f32)std::atof(src); }
internal void _convert(const char *src, i32 *loc) { *loc = std::atoi(src); }

#define command_parse_args1(tokens, loc1) { _convert((tokens)[1].start, loc1); }
#define command_parse_args2(tokens, loc1, loc2) { _convert((tokens)[1].start, loc1); _convert((tokens)[2].start, loc2); }

internal void add_command(StaticArray<Command> *commands, const char *name, u32 arg_count, void (*proc)(CommandToken *args, u32 arg_count)) {
    Command *command = commands->allocate_item();

    strncpy_s(command->name, name, STATIC_ARRAY_CAPACITY(command->name));
    command->name_size = (u16)strlen(name);
    command->arg_count = arg_count;
    command->proc = proc;
}

internal void init_commands(StaticArray<Command> *commands) {
    add_command(commands, "ping", 0, command_ping);
    add_command(commands, "add", 2, command_add);
    add_command(commands, "save", 0, command_save);
    add_command(commands, "load", 1, command_load);
    add_command(commands, "new_level", 1, command_new_level);
    add_command(commands, "clear", 0, command_new_level);
}

internal void execute_command(StaticArray<Command> *commands, char *input) {
    CommandToken tokens[8];
    u32 token_count = 0;

    // Split string
    {
        char *walker = input;
        while (*walker) {
            walker = eat_spacing(walker);
            char *start = walker;
            walker = eat_until_whitespace(walker);
            size_t length = walker - start;

            CommandToken *new_token = &tokens[token_count];
            new_token->start = start;
            new_token->length = length;
            token_count += 1;
        }
    }

    // It must always be at least one (command name)
    assert(token_count > 0);

    char *command_name = tokens[0].start;


    u32 arg_count = (u32)(token_count - 1); // -1 because zero index is the command name
    bool found = false;

    Command *it;
    array_foreach(*commands, it) {
        size_t compare_length = max(tokens[0].length, it->name_size);

        if (strncmp(command_name, it->name, compare_length) == 0) {
            found = true;

            if (arg_count != it->arg_count) {
                console_add_to_history("Invalid argument count: got %d expected %d", arg_count, it->arg_count);
                break;
            }

            it->proc(tokens, token_count);
        }
    }

    if (!found) {
        console_add_to_history("Command not found: %.*s", tokens[0].length, command_name);
    }
}

#endif
