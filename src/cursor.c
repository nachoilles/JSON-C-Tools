#include "cursor.h"
#include "tokenizer.h"


static inline int has_error(Cursor* p_c) {
  return p_c->error[0] != '\0';
}

void init(Cursor* p_c, Token** p_tokens, size_t count) {
  p_c->p_tokens = p_tokens;
  p_c->count = count;
  p_c->pos = 0;
  p_c->error[0] = '\0';
}

Token* peek(Cursor* p_c) {
  if (is_at_end(p_c)) return NULL;
  return p_c->p_tokens[p_c->pos];
}

Token* advance(Cursor* p_c) {
  if (is_at_end(p_c)) return NULL;
  return p_c->p_tokens[p_c->pos++];
}

Token* previous(Cursor* p_c) {
  if (p_c->pos == 0) return NULL;
  return p_c->p_tokens[p_c->pos - 1];
}

int is_at_end(Cursor* p_c) {
  return p_c->pos >= p_c->count;
}

int match(Cursor* p_c, uint8_t type) {
  if (is_at_end(p_c)) return 0;

  if (peek(p_c)->type != type) return 0;

  advance(p_c);
  return 1;
}

Token* expect(Cursor* p_c, uint8_t type) {
  if (is_at_end(p_c)) {
    snprintf(p_c->error, sizeof(p_c->error), 
      "[CURSOR] Error: unexpected end of input\n");
    return NULL;
  }

  Token* tk = peek(p_c);

  if (tk->type != type) {
    snprintf(p_c->error, sizeof(p_c->error), 
      "[CURSOR] Error: expected token type %u, got %u\n", type, tk->type);
    return NULL;
  }

  return advance(p_c);
}

size_t position(Cursor* p_c) {
  return p_c->pos;
}

void reset(Cursor* p_c, size_t pos) {
  if (pos <= p_c->count) {
    p_c->pos = pos;
  }
}
