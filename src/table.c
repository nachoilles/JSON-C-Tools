#include "table.h"
#include "tokenizer.h"


void init_parsing_table(int table[NT_COUNT][T_COUNT]) {
  for (int i = 0; i < NT_COUNT; i++)
    for (int j = 0; j < T_COUNT; j++)
      table[i][j] = -1;

  table[NT_JSON][T_STRING]   = 1;
  table[NT_JSON][T_NUMBER]   = 1;
  table[NT_JSON][T_LITERAL]  = 1;
  table[NT_JSON][T_LBRACE]   = 1;
  table[NT_JSON][T_LBRACKET] = 1;

  table[NT_ELEMENT][T_STRING]   = 4;
  table[NT_ELEMENT][T_NUMBER]   = 5;
  table[NT_ELEMENT][T_LITERAL]  = 6;
  table[NT_ELEMENT][T_LBRACE]   = 2;
  table[NT_ELEMENT][T_LBRACKET] = 3;

  table[NT_OBJECT][T_LBRACE] = 7;

  table[NT_MEMBERS_OPT][T_STRING] = 8;
  table[NT_MEMBERS_OPT][T_RBRACE] = 9;

  table[NT_MEMBERS][T_STRING] = 10;

  table[NT_MEMBERS_TAIL][T_COMMA] = 11;
  table[NT_MEMBERS_TAIL][T_RBRACE] = 12;

  table[NT_MEMBER][T_STRING] = 13;

  table[NT_ARRAY][T_LBRACKET] = 14;

  table[NT_ELEMENTS_OPT][T_STRING]   = 15;
  table[NT_ELEMENTS_OPT][T_NUMBER]   = 15;
  table[NT_ELEMENTS_OPT][T_LITERAL]  = 15;
  table[NT_ELEMENTS_OPT][T_LBRACE]   = 15;
  table[NT_ELEMENTS_OPT][T_LBRACKET] = 15;
  table[NT_ELEMENTS_OPT][T_RBRACKET] = 16;

  table[NT_ELEMENTS][T_STRING]   = 17;
  table[NT_ELEMENTS][T_NUMBER]   = 17;
  table[NT_ELEMENTS][T_LITERAL]  = 17;
  table[NT_ELEMENTS][T_LBRACE]   = 17;
  table[NT_ELEMENTS][T_LBRACKET] = 17;

  table[NT_ELEMENTS_TAIL][T_COMMA]    = 18;
  table[NT_ELEMENTS_TAIL][T_RBRACKET] = 19;
}

TokenClass classify(Token* t) {
  switch (t->type) {
    case LITERAL:
      return T_LITERAL;
    case STRING:
      return T_STRING;
    case NUMBER:
      return T_NUMBER;
    case PUNCTUATOR: {
      char c = t->p_value[0];
      if (c == '{') return T_LBRACE;
      if (c == '}') return T_RBRACE;
      if (c == '[') return T_LBRACKET;
      if (c == ']') return T_RBRACKET;
      if (c == ',') return T_COMMA;
      return T_DOLLAR;
    }
    default: return T_DOLLAR;
  }
}