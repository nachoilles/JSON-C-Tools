#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../src/tokenizer.h"
#include "../src/cursor.h"


#define ERROR_SIZE 4096
#define ASSERT(cond, msg) \
    do { if (!(cond)) {\
      fprintf(stderr, "\033[31m%s\x1b[37m\n", msg);\
      exit(1);\
    } } while (0)
#define SUCCESS(test) printf("[%s] \033[38;5;156msuccess\x1b[37m\n", test)


static Token* make_token(uint8_t type) {
    Token* tk = malloc(sizeof(Token));

    tk->type = type;
    tk->start = 0;
    tk->size = 0;
    tk->p_value = NULL;

    return tk;
}

static void test_init(void) {
  Token** p_tokens = malloc(2 * sizeof(Token*));
  p_tokens[0] = make_token(LITERAL);
  p_tokens[1] = make_token(STRING);

  Cursor c;
  init(&c, p_tokens, 2);

  ASSERT(c.p_tokens == p_tokens,
    "[test_init] invalid token array pointer");

  ASSERT(c.count == 2,
    "[test_init] invalid count");

  ASSERT(c.pos == 0,
    "[test_init] invalid initial position");

  ASSERT(c.error[0] == '\0',
    "[test_init] unexpected error");

  free_tokens(p_tokens, 2);
  SUCCESS("test_init");
}

static void test_peek(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 2);

  p_tokens[0] = make_token(LITERAL);
  p_tokens[1] = make_token(STRING);

  Cursor c;
  init(&c, p_tokens, 2);

  Token* tk = peek(&c);

  ASSERT(tk != NULL,
    "[test_peek] peek returned NULL");

  ASSERT(tk->type == LITERAL,
    "[test_peek] invalid token type");

  ASSERT(c.pos == 0,
    "[test_peek] peek modified cursor");

  tk = NULL;
  free_tokens(p_tokens, 2);
  SUCCESS("test_peek");
}

static void test_advance(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 2);

  p_tokens[0] = make_token(STRING);
  p_tokens[1] = make_token(LITERAL);

  Cursor c;
  init(&c, p_tokens, 2);

  Token* tk1 = advance(&c);

  ASSERT(tk1->type == STRING,
    "[test_advance] first token mismatch");

  ASSERT(c.pos == 1,
    "[test_advance] cursor did not advance");

  Token* tk2 = advance(&c);

  ASSERT(tk2->type == LITERAL,
    "[test_advance] second token mismatch");

  ASSERT(c.pos == 2,
    "[test_advance] invalid final position");

  tk1 = NULL;
  tk2 = NULL;
  free_tokens(p_tokens, 2);
  SUCCESS("test_advance");
}

static void test_previous(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 2);

  p_tokens[0] = make_token(LITERAL);
  p_tokens[1] = make_token(STRING);

  Cursor c;
  init(&c, p_tokens, 2);

  ASSERT(previous(&c) == NULL,
    "[test_previous] expected NULL at pos 0");

  advance(&c);

  Token* tk = previous(&c);

  ASSERT(tk != NULL,
    "[test_previous] previous returned NULL");

  ASSERT(tk->type == LITERAL,
    "[test_previous] invalid previous token");

  tk = NULL;
  free_tokens(p_tokens, 2);
  SUCCESS("test_previous");
}

static void test_is_at_end(void) {
  Token** p_tokens = malloc(sizeof(Token*));

  p_tokens[0] = make_token(LITERAL);

  Cursor c;
  init(&c, p_tokens, 1);

  ASSERT(is_at_end(&c) == 0,
    "[test_is_at_end] expected not at end");

  advance(&c);

  ASSERT(is_at_end(&c) == 1,
    "[test_is_at_end] expected at end");

  free_tokens(p_tokens, 1);
  SUCCESS("test_is_at_end");
}

static void test_match(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 2);

  p_tokens[0] = make_token(LITERAL);
  p_tokens[1] = make_token(STRING);

  Cursor c;
  init(&c, p_tokens, 2);

  ASSERT(match(&c, LITERAL) == 1,
    "[test_match] expected successful match");

  ASSERT(c.pos == 1,
    "[test_match] cursor did not advance");

  ASSERT(match(&c, LITERAL) == 0,
    "[test_match] unexpected successful match");

  ASSERT(c.pos == 1,
    "[test_match] cursor advanced incorrectly");

  free_tokens(p_tokens, 2);
  SUCCESS("test_match");
}

static void test_expect_success(void) {
  Token** p_tokens = malloc(sizeof(Token*));

  p_tokens[0] = make_token(LITERAL);

  Cursor c;
  init(&c, p_tokens, 1);

  Token* tk = expect(&c, LITERAL);

  ASSERT(tk != NULL,
    "[test_expect_success] expect returned NULL");

  ASSERT(c.pos == 1,
    "[test_expect_success] cursor did not advance");

  printf("%s", c.error);
  ASSERT(c.error[0] == '\0',
    "[test_expect_success] unexpected error");

  tk = NULL;
  free_tokens(p_tokens, 1);
  SUCCESS("test_expect_success");
}

static void test_expect_failure(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 1);

  p_tokens[0] = make_token(LITERAL);

  Cursor c;
  init(&c, p_tokens, 1);

  Token* tk = expect(&c, STRING);

  ASSERT(tk == NULL,
    "[test_expect_failure] expected NULL");

  ASSERT(c.error[0] != '\0',
    "[test_expect_failure] missing error message");

  tk = NULL;
  free_tokens(p_tokens, 1);
  SUCCESS("test_expect_failure");
}

static void test_expect_eof(void) {
  Cursor c;

  init(&c, NULL, 0);

  Token* tk = expect(&c, 1);

  ASSERT(tk == NULL,
    "[test_expect_eof] expected NULL");

  ASSERT(c.error[0] != '\0',
    "[test_expect_eof] missing eof error");

  tk = NULL;
  free_token(tk);
  SUCCESS("test_expect_eof");
}

static void test_reset(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 3);

  p_tokens[0] = make_token(LITERAL);
  p_tokens[1] = make_token(STRING);
  p_tokens[2] = make_token(NUMBER);

  Cursor c;
  init(&c, p_tokens, 3);

  advance(&c);
  advance(&c);

  ASSERT(c.pos == 2,
      "[test_reset] invalid position before reset");

  reset(&c, 1);

  ASSERT(c.pos == 1,
      "[test_reset] reset failed");

  reset(&c, 100);

  ASSERT(c.pos == 1,
      "[test_reset] invalid reset beyond bounds");

  free_tokens(p_tokens, 3);
  SUCCESS("test_reset");
}

static void test_position(void) {
  Token** p_tokens = malloc(sizeof(Token*) * 2);

  p_tokens[0] = make_token(LITERAL);
  p_tokens[1] = make_token(STRING);

  Cursor c;
  init(&c, p_tokens, 2);

  ASSERT(position(&c) == 0,
    "[test_position] invalid initial position");

  advance(&c);

  ASSERT(position(&c) == 1,
    "[test_position] invalid advanced position");

  free_tokens(p_tokens, 2);
  SUCCESS("test_position");
}

int main() {
  printf("========== CURSOR TEST ==========\n\n");

  test_init();
  test_peek();
  test_advance();
  test_previous();
  test_is_at_end();
  test_match();
  test_expect_success();
  test_expect_failure();
  test_expect_eof();
  test_reset();
  test_position();
  
  printf("\n[cursor_test] \033[38;5;156mALL TEST PASSED\x1b[37m\n\n");
}
