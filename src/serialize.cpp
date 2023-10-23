internal bool parse_f32_1(Tokenizer *tokenizer, f32 *value) {
    Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
    if (tokenizer->error) return false;

    *value = number.parsed_number_f32;

    return true;
}

internal bool parse_f32_2(Tokenizer *tokenizer, f32 *value_a, f32 *value_b) {
    {
        Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
        if (tokenizer->error) return false;

        *value_a = number.parsed_number_f32;
    }

    tokenizer_require_token(tokenizer, TokenVariant::Comma);
    if (tokenizer->error) return false;

    {
        Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
        if (tokenizer->error) return false;

        *value_b = number.parsed_number_f32;
    }

    return true;
}

internal bool parse_i32_1(Tokenizer *tokenizer, i32 *value) {
    Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
    if (tokenizer->error) return false;

    *value = number.parsed_number_i32;

    return true;
}

internal bool parse_i32_2(Tokenizer *tokenizer, i32 *value_a, i32 *value_b) {
    {
        Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
        if (tokenizer->error) return false;

        *value_a = number.parsed_number_i32;
    }

    tokenizer_require_token(tokenizer, TokenVariant::Comma);
    if (tokenizer->error) return false;

    {
        Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
        if (tokenizer->error) return false;

        *value_b = number.parsed_number_i32;
    }

    return true;
}

internal bool parse_u32_1(Tokenizer *tokenizer, u32 *value) {
    Token number = tokenizer_require_token(tokenizer, TokenVariant::Number);
    if (tokenizer->error) return false;

    i32 parsed_number = number.parsed_number_i32;
    if (parsed_number < 0) parsed_number = 0;

    *value = parsed_number;

    return true;
}
