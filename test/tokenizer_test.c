#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../src/tokenizer.h"


#define ERROR_SIZE 4096
#define ASSERT(cond, msg) \
    do { if (!(cond)) {\
      fprintf(stderr, "\033[31m%s\x1b[37m\n", msg);\
      exit(1);\
    } } while (0)
#define SUCCESS(test) printf("[%s] \033[38;5;156msuccess\x1b[37m\n", test)

static void test_empty_json(void) {
  char test_str[] = "";
  size_t tk_count = 0;
  char tokenizer_error[ERROR_SIZE];
  tokenizer_error[0] = '\0';

  Token** p_tokens = tokenize(
    test_str, 
    strlen(test_str), 
    &tk_count, 
    tokenizer_error, 
    ERROR_SIZE
  );

  char msg[256];

  snprintf(msg, sizeof(msg), 
    "[test_empty_json] expected 0 but got %d", tk_count);
  ASSERT(tk_count == strlen(test_str), msg);

  ASSERT(
    p_tokens == NULL, 
    "[test_empty_json] expected NULL"
  );

  ASSERT(
    tokenizer_error[0] != '\0', 
    "[test_empty_json] expected error"
  );

  free_tokens(p_tokens, tk_count);
  SUCCESS("test_empty_json");
}

static void test_empty_containers(void) {
  char* tests[] = { "{}", "[]" };
  size_t test_count = sizeof(tests) / sizeof(tests[0]);

  for (int t = 0; t < test_count; t++) {
    char* test_str = tests[t];
    size_t tk_count = 0;    
    char tokenizer_error[ERROR_SIZE];
    tokenizer_error[0] = '\0';
    
    Token** p_tokens = tokenize(
      test_str, 
      strlen(test_str), 
      &tk_count, 
      tokenizer_error, 
      ERROR_SIZE
    );
    
    char msg[256];

    snprintf(msg, sizeof(msg),
      "[test_empty_containers] expected 2 but got %d", tk_count);
    ASSERT(tk_count == strlen(test_str), msg);
    
    for (int i = 0; i < tk_count; i++) {
      snprintf(msg, sizeof(msg), 
        "[test_empty_containers] expected PUNCTUATOR but got %s", 
        token_type_to_string(p_tokens[i]->type));
      ASSERT(p_tokens[i]->type == PUNCTUATOR, msg);
      
      snprintf(msg, sizeof(msg),
        "[test_empty_containers] expected %c but got %c",
        test_str[i], p_tokens[i]->p_value[0]);
      ASSERT(p_tokens[i]->p_value[0] == test_str[i], msg);
    }

    ASSERT(
      tokenizer_error[0] == '\0', 
      "[test_empty_containers] expected empty string"
    );

    free_tokens(p_tokens, tk_count);
  }
  SUCCESS("test_empty_containers");
}

static void test_literals(void) {
  char* tests[] = { "null", "true", "false" };
  size_t test_count = sizeof(tests) / sizeof(tests[0]);

  for (int t = 0; t < test_count; t++) {
    char* test_str = tests[t];
    size_t tk_count = 0;    
    char tokenizer_error[ERROR_SIZE];
    tokenizer_error[0] = '\0';
    
    Token** p_tokens = tokenize(
      test_str, 
      strlen(test_str), 
      &tk_count, 
      tokenizer_error, 
      ERROR_SIZE
    );

    char msg[256];

    snprintf(msg, sizeof(msg),
      "[test_literals] expected 1 but got %d", tk_count);
    ASSERT(tk_count == 1, msg);

    snprintf(msg, sizeof(msg),
      "[test_literals] expected LITERAL but got %s", 
      token_type_to_string(p_tokens[0]->type));
    ASSERT(p_tokens[0]->type == LITERAL, msg);

    snprintf(msg, sizeof(msg),
      "[test_literals] expected %s but got %s",
      test_str, p_tokens[0]->p_value);
    ASSERT(strcmp(test_str, p_tokens[0]->p_value) == 0, msg);

    snprintf(msg, sizeof(msg),
      "[test_literals] expected \"\" but got %s",
      tokenizer_error);
    ASSERT(strcmp(test_str, p_tokens[0]->p_value) == 0, msg);

    ASSERT(
      tokenizer_error[0] == '\0', 
      "[test_literals] expected empty string"
    );

    free_tokens(p_tokens, tk_count);
  }
  SUCCESS("test_literals");
}

static void test_valid_numbers(void) {
  char* tests[] = { "0", "-0", "0.1", "0.01", 
    "-1", "1.23", "1E23", "1.2e-3", "1e+02",
    "0E0", "0e-0" };
  size_t test_count = sizeof(tests) / sizeof(tests[0]);

  for (int t = 0; t < test_count; t++) {
    char* test_str = tests[t];
    size_t tk_count = 0;    
    char tokenizer_error[ERROR_SIZE];
    tokenizer_error[0] = '\0';
    
    Token** p_tokens = tokenize(
      test_str, 
      strlen(test_str), 
      &tk_count, 
      tokenizer_error, 
      ERROR_SIZE
    );

    char msg[256];

    snprintf(msg, sizeof(msg),
      "[test_valid_numbers] expected 1 but got %d", tk_count);
    ASSERT(tk_count == 1, msg);

    snprintf(msg, sizeof(msg),
      "[test_valid_numbers] expected NUMBER but got %s", 
      token_type_to_string(p_tokens[0]->type));
    ASSERT(p_tokens[0]->type == NUMBER, msg);

    snprintf(msg, sizeof(msg),
      "[test_valid_numbers] expected %s but got %s",
      test_str, p_tokens[0]->p_value);
    ASSERT(strcmp(test_str, p_tokens[0]->p_value) == 0, msg);

    ASSERT(
      tokenizer_error[0] == '\0', 
      "[test_valid_numbers] expected empty string"
    );

    free_tokens(p_tokens, tk_count);
  }
  SUCCESS("test_valid_numbers");
}

static void test_invalid_numbers(void) {
  char* tests[] = { "00", "01", "0.", 
    "1.", "1e", "1E", "--1", "+1", "--" };
  size_t test_count = sizeof(tests) / sizeof(tests[0]);

  for (int t = 0; t < test_count; t++) {
    char* test_str = tests[t];

    size_t tk_count = 0;    
    char tokenizer_error[ERROR_SIZE];
    tokenizer_error[0] = '\0';
    
    Token** p_tokens = tokenize(
      test_str, 
      strlen(test_str), 
      &tk_count, 
      tokenizer_error, 
      ERROR_SIZE
    );

    char msg[256];

    snprintf(msg, sizeof(msg),
      "[test_invalid_numbers] expected NULL for input: %s",
      test_str);
    ASSERT(p_tokens == NULL, msg);

    ASSERT(tokenizer_error[0] != '\0', "[test_invalid_numbers] expected error");

    free_tokens(p_tokens, tk_count);
  }
  SUCCESS("test_invalid_numbers");
}

static void test_valid_strings(void) {
  char* tests[] = { 
    "\"\"",
    "\"hello\"",
    "\"Hello, World!\"",
    "\"123\"",
    "\"true\"",
    "\"null\"",

    /* whitespace */
    "\" \"",
    "\"\\t\"",
    "\"\\n\"",
    "\"\\r\\n\"",

    /* escapes */
    "\"\\\\\"",
    "\"\\\"\"",
    "\"\\/\"",
    "\"\\b\"",
    "\"\\f\"",
    "\"\\n\"",
    "\"\\r\"",
    "\"\\t\"",

    /* mixed escapes */
    "\"line1\\nline2\"",
    "\"tab\\tindent\"",
    "\"quote: \\\"\"",
    "\"backslash: \\\\\"",

    /* unicode */
    "\"\\u0041\"",
    "\"\\u03A9\"",
    "\"\\u2764\"",

    /* punctuation */
    "\"{}[],:\"",

    /* spaces */
    "\"   spaced   \"",

    /* long */
    "\"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\"" 
  };
  size_t test_count = sizeof(tests) / sizeof(tests[0]);

  for (int t = 0; t < test_count; t++) {
    char* test_str = tests[t];

    size_t tk_count = 0;    
    char tokenizer_error[ERROR_SIZE];
    tokenizer_error[0] = '\0';
    
    Token** p_tokens = tokenize(
      test_str, 
      strlen(test_str), 
      &tk_count, 
      tokenizer_error, 
      ERROR_SIZE
    );

    char msg[8192];

    snprintf(msg, sizeof(msg),
      "[test_valid_strings] expected 1 but got %d at input %zu",
      tk_count, t);
    ASSERT(tk_count == 1, msg);

    snprintf(msg, sizeof(msg),
      "[test_valid_strings] expected STRING but got %s",
      token_type_to_string(p_tokens[0]->type));
    ASSERT(p_tokens[0]->type == STRING, msg);

    size_t expected_len = strlen(test_str) - 2;
    snprintf(msg, sizeof(msg),
      "[test_valid_strings] expected %d but got %d",
      expected_len, strlen(p_tokens[0]->p_value));
    ASSERT(strlen(p_tokens[0]->p_value) == expected_len, msg);
  
    snprintf(msg, sizeof(msg),
      "[test_valid_strings] expected %s but got \"%s\"",
      test_str, p_tokens[0]->p_value);
    ASSERT(strncmp(p_tokens[0]->p_value, test_str + 1, expected_len) == 0, msg);

    ASSERT(
      tokenizer_error[0] == '\0', 
      "[test_valid_numbers] expected empty string"
    );
    
    free_tokens(p_tokens, tk_count);
  }
  SUCCESS("test_valid_strings");
}

static void test_invalid_strings(void) {
  char* tests[] = {
    "\"", "\"abc", "\"\\a\"", "\"\\c\"", "\"\\d\"", "\"\\e\"", "\"\\g\"", "\"\\h\"", "\"\\i\"", 
    "\"\\j\"", "\"\\k\"", "\"\\l\"", "\"\\m\"", "\"\\o\"", "\"\\p\"", "\"\\q\"", "\"\\s\"", 
    "\"\\v\"", "\"\\w\"", "\"\\x\"", "\"\\y\"", "\"\\z\"", "\"\\A\"", "\"\\B\"", "\"\\C\"", "\"\\D\"", 
    "\"\\E\"", "\"\\F\"", "\"\\G\"", "\"\\H\"", "\"\\I\"", "\"\\J\"", "\"\\K\"", "\"\\L\"", "\"\\M\"", 
    "\"\\N\"", "\"\\O\"", "\"\\P\"", "\"\\Q\"", "\"\\R\"", "\"\\S\"", "\"\\T\"", "\"\\U\"", "\"\\V\"", 
    "\"\\W\"", "\"\\X\"", "\"\\Y\"", "\"\\Z\"", "\"\\x41\"", "\"\\u12\"", "\"\\u123\"", "\"\\uGGGG\"", 
    "\"\n\"", "\"\r\"", "\"\t\"", "\"\\", "\"\\\\\\\"",
  };
  size_t test_count = sizeof(tests) / sizeof(tests[0]);

  for (size_t t = 0; t < test_count; t++) {
    char* test_str = tests[t];

    size_t tk_count = 0;
    char tokenizer_error[ERROR_SIZE] = {0};
    tokenizer_error[0] = '\0';

    Token** p_tokens = tokenize(
      test_str,
      strlen(test_str),
      &tk_count,
      tokenizer_error,
      ERROR_SIZE
    );

    char msg[256];

    snprintf(msg, sizeof(msg),
      "[test_invalid_strings] expected error for test %zu",
      t);
    ASSERT(tokenizer_error[0] != '\0', msg);

    free_tokens(p_tokens, tk_count);
  }
  SUCCESS("test_invalid_strings");
}

int main() {
  printf("========== TOKENIZER TEST ==========\n\n");

  test_empty_json();
  test_empty_containers();
  test_literals();
  test_valid_numbers();
  test_invalid_numbers();
  test_valid_strings();
  test_invalid_strings();

  printf("\n[tokenizer_test] \033[38;5;156mALL TEST PASSED\x1b[37m\n\n");
  return 0;
}