internal void tokenizer_refill(Tokenizer *tokenizer) {
    if (tokenizer->input_length == 0) {
        tokenizer->at[0] = 0;
        tokenizer->at[1] = 0;
    }
    else if (tokenizer->input_length == 1) {
        tokenizer->at[0] = tokenizer->input[0];
        tokenizer->at[1] = 0;
    }
    else {
        tokenizer->at[0] = tokenizer->input[0];
        tokenizer->at[1] = tokenizer->input[1];
    }
}

internal void tokenizer_advance(Tokenizer *tokenizer, u32 count) {
    if (count > tokenizer->input_length) {
        tokenizer->input += tokenizer->input_length;
        tokenizer->input_length = 0;
    }
    else {
        tokenizer->input += count;
        tokenizer->input_length -= count;
    }

    tokenizer_refill(tokenizer);
}

internal Token tokenizer_get_raw_token(Tokenizer *tokenizer) {
    Token token = {};
    token.text = tokenizer->input;

    char first_char = tokenizer->at[0];
    tokenizer_advance(tokenizer, 1);

    switch (first_char) {
        case '\0': { token.type = TokenVariant::EndOfStream; } break;

        case '(': { token.type = TokenVariant::OpenParen; } break;
        case ')': { token.type = TokenVariant::CloseParen; } break;
        case '[': { token.type = TokenVariant::OpenBracket; } break;
        case ']': { token.type = TokenVariant::CloseBracket; } break;
        case '{': { token.type = TokenVariant::OpenBrace; } break;
        case '}': { token.type = TokenVariant::CloseBrace; } break;
        case ':': { token.type = TokenVariant::Colon; } break;
        case ',': { token.type = TokenVariant::Comma; } break;
        case ';': { token.type = TokenVariant::Semicolon; } break;

        case '"' : {
            token.type = TokenVariant::String;

            u32 start_line = tokenizer->line_number;

            while (tokenizer->at[0] && tokenizer->at[0] != '"') {
                tokenizer_advance(tokenizer, 1);
            }

            if (tokenizer->at[0] == '"') {
                tokenizer_advance(tokenizer, 1);
            }
            else {
                tokenizer->error = true;
                log_print("Error: file contains unclosed string (starting at line %d)\n", start_line);
            }
        } break;

        default: {
            if (is_end_of_line(first_char)) {
                token.type = TokenVariant::EndOfLine;

                if ((first_char == '\n' && tokenizer->at[0] == '\r') || (first_char == '\r' && tokenizer->at[0] == '\n')) {
                    tokenizer_advance(tokenizer, 1);
                }

                tokenizer->line_number += 1;
            }
            else if (is_space(first_char)) {
                token.type = TokenVariant::Spacing;

                while (is_space(tokenizer->at[0])) {
                    tokenizer_advance(tokenizer, 1);
                }
            }
            else if (is_alpha(first_char)) {
                token.type = TokenVariant::Identifier;

                while (is_alpha(tokenizer->at[0]) || is_numeric(tokenizer->at[0]) || tokenizer->at[0] == '_') {
                    tokenizer_advance(tokenizer, 1);
                }
            }
            else if (is_numeric(first_char) || (first_char == '-' && is_numeric(tokenizer->at[0]))) {
                token.type = TokenVariant::Number;

                bool is_negative = false;

                if (first_char == '-') {
                    is_negative = true;

                    first_char = tokenizer->at[0];
                    tokenizer_advance(tokenizer, 1);
                }

                f32 parsed_number = (f32)(first_char - '0');

                while (is_numeric(tokenizer->at[0])) {
                    f32 digit = (f32)(tokenizer->at[0] - '0');

                    parsed_number *= 10.0f;
                    parsed_number += digit;

                    tokenizer_advance(tokenizer, 1);
                }

                if (tokenizer->at[0] == '.') {
                    tokenizer_advance(tokenizer, 1);

                    f32 decimal_point = 0.1f;

                    while (is_numeric(tokenizer->at[0])) {
                        f32 digit = (f32)(tokenizer->at[0] - '0');

                        parsed_number += decimal_point * digit;
                        decimal_point *= 0.1f;

                        tokenizer_advance(tokenizer, 1);
                    }
                }

                if (is_negative) {
                    parsed_number *= -1.0f;
                }

                token.parsed_number_f32 = parsed_number;
                token.parsed_number_i32 = (i32)parsed_number;
            }
            else if (first_char == '#') {
                token.type = TokenVariant::Comment;

                while (tokenizer->at[0] && !is_end_of_line(tokenizer->at[0])) {
                    tokenizer_advance(tokenizer, 1);
                }
            }
            else {
                token.type = TokenVariant::Unknown;
            }
        }
    }

    token.length = (tokenizer->input - token.text);

    return token;
}

internal Token tokenizer_get_real_token(Tokenizer *tokenizer) {
    Token token = {};

    while (1) {
        token = tokenizer_get_raw_token(tokenizer);

        if (token.type == TokenVariant::Comment || token.type == TokenVariant::Spacing || token.type == TokenVariant::EndOfLine) {
            continue;
        }

        if (token.type == TokenVariant::String) {
            if (token.length != 0 && token.text[0] == '"') {
                token.length -= 1;
                token.text += 1;
            }

            if (token.length != 0 && token.text[token.length - 1] == '"') {
                token.length -= 1;
            }
        }

        break;
    }

    return token;
}

internal bool token_matches(Token token, char *to_match) {
    size_t to_match_length = strlen(to_match);

    size_t check_size = max(to_match_length, token.length);

    bool match = strncmp(token.text, to_match, check_size) == 0;

    return match;
}

internal Token tokenizer_require_token(Tokenizer *tokenizer, TokenVariant required_type) {
    Token token = tokenizer_get_real_token(tokenizer);

    if (token.type != required_type) {
        log_print(
            "Error: tokenizer wrong token. Required '%s', got '%s' on line %d\n",
            TOKEN_VARIANT_STRINGS[(u32)required_type],
            TOKEN_VARIANT_STRINGS[(u32)token.type],
            tokenizer->line_number
        );
        tokenizer->error = true;
    }

    return token;
}

internal Tokenizer tokenize(char *input, size_t input_length) {
    Tokenizer tokenizer = {};
    tokenizer.input = input;
    tokenizer.input_length = input_length;
    tokenizer_refill(&tokenizer);

    return tokenizer;
}
