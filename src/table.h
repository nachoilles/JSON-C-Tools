#ifndef TABLE
#define TABLE


#include <string.h>

// Forward declaration. Implemented in tokenizer.h
typedef struct Token Token;

typedef enum {
  NT_JSON,
  NT_ELEMENT,
  NT_OBJECT,
  NT_MEMBERS_OPT,
  NT_MEMBERS,
  NT_MEMBERS_TAIL,
  NT_MEMBER,
  NT_ARRAY,
  NT_ELEMENTS_OPT,
  NT_ELEMENTS,
  NT_ELEMENTS_TAIL,
  NT_COUNT
} NonTerminal;

typedef enum {
  T_STRING,
  T_NUMBER,
  T_LITERAL,
  T_LBRACE,
  T_LBRACKET,
  T_RBRACE,
  T_RBRACKET,
  T_COMMA,
  T_DOLLAR,
  T_COUNT
} TokenClass;

void init_parsing_table(int table[NT_COUNT][T_COUNT]);

TokenClass classify(Token* t);

#endif //TABLE