#ifndef CURSOR
#define CURSOR

#include <stdint.h>
#include "globals.h"


// Forward declaration. Definition in tokenizer.h
typedef struct Token Token;

typedef struct {
  Token** p_tokens;
  size_t count;
  size_t pos;
  char error[ERROR_BUFFER_SIZE];
} Cursor;

static inline int has_error(Cursor* p_c);

void init(Cursor* p_c, Token** p_tokens, size_t count);

Token* peek(Cursor* p_c);
Token* advance(Cursor* p_c);
Token* previous(Cursor* p_c);
int is_at_end(Cursor* p_c);

int match(Cursor* p_c, uint8_t type);
Token* expect(Cursor* p_c, uint8_t type);

size_t position(Cursor* p_c);
void reset(Cursor* p_c, size_t pos);

#endif // CURSOR