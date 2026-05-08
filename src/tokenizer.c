#include  "tokenizer.h"


const char* token_type_to_string(const uint8_t type) {
  switch (type) {
    case LITERAL: return "LITERAL";
    case STRING: return "STRING";
    case NUMBER: return "NUMBER";
    case PUNCTUATOR: return "PUNCTUATOR";
    default: return "\x1b[31mUNKNOWN\x1b[37m";
  }
}

Token* create_token(
  uint8_t type,
  uint64_t start,
  size_t size,
  const char* p_value
) {
  Token* s = malloc(sizeof(Token));
  if (!s) return NULL;
  
  s->type = type;
  s->start = start;
  s->size = size;

  s->p_value = malloc(size + 1);
  if (!s->p_value) {
    free(s);
    return NULL;
  }
  memcpy(s->p_value, p_value, size);
  s->p_value[size] = '\0';

  return s;
}

void free_token(Token* p_token) {
  if (!p_token) return;
  free(p_token->p_value);
  free(p_token);
}

_Bool is_whitespace(const char c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

_Bool is_punctuator(const char c) {
  return c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',';
}

_Bool is_digit(const char c) {
  return c >= '0' && c <= '9';
}

void free_tokens(Token** p_tokens, const int count) {
  for (int i = 0; i < count; i++) {
    free_token(p_tokens[i]);
  }
  free(p_tokens);
}

Token** tokenize(
  const char* str, 
  size_t len, 
  size_t* p_tk_size, 
  char* error, 
  size_t e_size
) {
  if (p_tk_size != NULL) {
    *p_tk_size = 0;
  }

  if (error != NULL && e_size > 0) {
    error[0] = '\0';
  }

  if (str == NULL) {
    if (error && e_size > 0) {
      snprintf(error, e_size, "[TOKENIZER] Error: input string is NULL\n");
    }
    return NULL;
  }

  if (len == 0) {
    if (error && e_size > 0) {
      snprintf(error, e_size, "[TOKENIZER] Error: empty input\n");
    }
    return NULL;
  }

  int stack_capacity = 50;
  int count = 0;

  Token** p_tokens = malloc(sizeof(Token*) * stack_capacity);
  if(!p_tokens) return NULL;

  int i = 0;

  while(i < len) {
    // Re_size token buffer if necessary
    if (count >= stack_capacity) {
      stack_capacity *= 2;
      Token** tmp = realloc(p_tokens, sizeof(Token*) * stack_capacity);
      if (!tmp) {
        char e[] = "[TOKENIZER] Error: couldn't allocate p_tokens\n";
        snprintf(error, e_size, "%s", e);
        free_tokens(p_tokens, count);
        return NULL;
      }
      p_tokens = tmp;
    }

    // Ignore whitespace
    if (is_whitespace(str[i])) {
      i++;
      continue;
    }

    // Handle punctuators
    else if (is_punctuator(str[i])) {
      Token* tk = create_token(PUNCTUATOR, i, 1, &str[i]);
      p_tokens[count++] = tk;
      i++;
      continue;
    }

    // Handle literals
    else if (i + 4 <= len && strncmp(&str[i], "true", 4) == 0) {
      Token* tk = create_token(LITERAL, i, 4, &str[i]);
      p_tokens[count++] = tk;
      i += 4;
      continue;
    }

    else if (i + 5 <= len && strncmp(&str[i], "false", 5) == 0) {
      Token* tk = create_token(LITERAL, i, 5, &str[i]);
      p_tokens[count++] = tk;
      i += 5;
      continue;
    }

    else if (i + 4 <= len && strncmp(&str[i], "null", 4) == 0) {
      Token* tk = create_token(LITERAL, i, 4, &str[i]);
      p_tokens[count++] = tk;
      i += 4;
      continue;
    }

    // Handle strings
    else if (str[i] == '"') {
      size_t start = i + 1;
      i++;

      int escaped = 0;
      int escaped_u = 0;

      while(i < len) {
        char c = str[i];

        if (escaped_u > 0) {
          if ((c >= '0' && c <= '9') || 
              (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F')) {
            escaped_u += 1;
            if (escaped_u > 4) {
                escaped_u = 0;
            }
            i++;
            continue;
          } else {
            char e[] = "[TOKENIZER] Error: invalid escape seuence\n";
            snprintf(error, e_size, "%s", e);
            free_tokens(p_tokens, count);
            return NULL;
          }
        }

        if (escaped) {
          if (c != '"' && c != '\\' && c != '/' &&
              c != 'b' && c != 'f' && c != 'n' &&
              c != 'r' && c != 't' && c != 'u') {
            char e[] = "[TOKENIZER] Error: invalid escape sequence\n";
            snprintf(error, e_size, "%s", e);
            free_tokens(p_tokens, count);
            return NULL;
          }

          if (c == 'u') {
            escaped_u = 1;
          }

          escaped = 0;
          i++;
          continue;
        }

        if (c == '\\') {
          escaped = 1;
          i++;
          continue;
        }

        if (c== '"') {
          break;
        }

        if ((unsigned char)c < 0x20) {
          char e[] = "[TOKENIZER] Error: unescaped control character in string\n";
          snprintf(error, e_size, "%s", e);
          free_tokens(p_tokens, count);
          return NULL;
        }

        i++;
      }

      if (i >= len || str[i] != '"') {
        char e[] = "[TOKENIZER] Error: unterminated string\n";
        snprintf(error, e_size, "%s", e);
        free_tokens(p_tokens, count);
        return NULL;
      }

      size_t strLen = i - start;
      Token* tk = create_token(STRING, start - 1, strLen, &str[start]);

      p_tokens[count++] = tk;
      i++;
  }

    // Handle numbers
    else if (is_digit(str[i]) || str[i] == '-') {
      size_t start = i;
      if (str[i] == '-') i++;

      // Integer part
      if (i >= len || !is_digit(str[i])) {
        char e[] = "[TOKENIZER] Error: invalid number format\n";
        snprintf(error, e_size, "%s", e);
        free_tokens(p_tokens, count);
        return NULL;
      }

      /* Leading zero handling */
      if (str[i] == '0') {
        i++;

        /* Reject numbers like 01, 00, 0123 */
        if (i < len && is_digit(str[i])) {
          char e[] = "[TOKENIZER] Error: leading zeros are not allowed\n";
          snprintf(error, e_size, "%s", e);
          free_tokens(p_tokens, count);
          return NULL;
        }
      } else {
          while (i < len && is_digit(str[i])) {
            i++;
          }
      }

      // Fraction part
      if (i < len && str[i] == '.') {
        i++;
        int fracStart = i;

        while (i < len && is_digit(str[i])) {
          i++;
        }

        if (i == fracStart) {
          char e[] = "[TOKENIZER] Error: invalid fraction format\n";
          snprintf(error, e_size, "%s", e);
          free_tokens(p_tokens, count);
          return NULL;
        }
      }

      // Exponent part
      if (i < len && (str[i] == 'e' || str[i] == 'E')) {
        i++;

        if (i < len && (str[i] == '-' || str[i] == '+')) {
          i++;
        }

        int expStart = i;
        while (i < len && is_digit(str[i])) {
          i++;
        }

        if (i == expStart) {
          char e[] = "[TOKENIZER] Error: invalid exponent format\n";
          snprintf(error, e_size, "%d", e);
          free_tokens(p_tokens, count);
          return NULL;
        }
      }

      size_t numLen = i - start;
      Token* tk = create_token(NUMBER, start, numLen, &str[start]);
      p_tokens[count++] = tk;
    }

    // Malformed JSON
    else {
      snprintf(
        error, 
        e_size, 
        "[TOKENIZER] Error: unexpected character '%c' at position: %d\n", str[i], i
      );
      free_tokens(p_tokens, count);
      return NULL;
    }
  }

  *p_tk_size = count;
  return p_tokens;
}
