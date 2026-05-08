#ifndef TOKENIZER
#define TOKENIZER

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define PUNCTUATOR 0
#define LITERAL 1
#define STRING 2
#define NUMBER 3

typedef struct Token {
    uint8_t type;
    uint64_t start;
    size_t size;
    char* p_value;
} Token;

const char* token_type_to_string(const uint8_t type);

Token* create_token(uint8_t type, uint64_t start, size_t size, const char* p_value);

void free_token(Token* p_token);

_Bool is_whitespace(const char c);

_Bool is_punctuator(const char c);

_Bool is_digit(const char c);

void free_tokens(Token** p_tokens, const int count);

Token** tokenize(const char* str, size_t len, size_t* p_tk_count, char* error, size_t e_size);

#endif //TOKENIZER