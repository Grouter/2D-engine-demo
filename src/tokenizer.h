#if !defined(TOKENIZER_H)
#define TOKENIZER_H

const char* TOKEN_VARIANT_STRINGS[] = {
    "Unknown",

    "OpenParen",
    "CloseParen",
    "OpenBracket",
    "CloseBracket",
    "OpenBrace",
    "CloseBrace",
    "Colon",
    "Comma",
    "Semicolon",

    "Identifier",
    "String",
    "Number",

    "Spacing",
    "Comment",
    "EndOfLine",
    "EndOfStream",
};

// This cannot be called TokenType thanks to win32 API <3
// PLus I can't be bothered with namespacing...
enum struct TokenVariant {
    Unknown,

    OpenParen,
    CloseParen,
    OpenBracket,
    CloseBracket,
    OpenBrace,
    CloseBrace,
    Colon,
    Comma,
    Semicolon,

    Identifier,
    String,
    Number,

    Spacing,
    Comment,
    EndOfLine,
    EndOfStream,
};

struct Token {
    TokenVariant type;

    char *text;
    size_t length;

    f32 parsed_number_f32;
    i32 parsed_number_i32;
};

struct Tokenizer {
    char *input;
    size_t input_length;

    u32 line_number = 1;
    bool32 error;

    char at[2];
};

#endif
