#include "../include/json.h"

#include "globals.h"
#include "table.h"
#include "cursor.h"
#include "tokenizer.h"



static int table[NT_COUNT][T_COUNT];

void json_set_debug(int enabled) {
  json_debug_enabled = enabled;
}

char* get_json_type_name(JsonType type) {
  switch (type) {
    case JSON_NULL: return "JSON_NULL";
    case JSON_BOOLEAN: return "JSON_BOOLEAN";
    case JSON_NUMBER: return "JSON_NUMBER";
    case JSON_STRING: return "JSON_STRING";
    case JSON_ARRAY: return "JSON_ARRAY";
    case JSON_OBJECT: return "JSON_OBJECT";
    default: return "UNKNOWN TYPE";
  }
}

JsonValue* json_null(void) {
  JsonValue* val = malloc(sizeof(JsonValue));
  val->type = JSON_NULL;
  return val;
}

JsonValue* json_boolean(bool val) {
  JsonValue* value = malloc(sizeof(JsonValue));
  value->type = JSON_BOOLEAN;
  value->value.boolean = val;
  return value;
}

JsonValue* json_number(double val) {
  JsonValue* value = malloc(sizeof(JsonValue));
  value->type = JSON_NUMBER;
  value->value.number = val;
  return value;
}

JsonValue* json_string(const char* val) {
  JsonValue* p_value = malloc(sizeof(JsonValue));
  p_value->type = JSON_STRING;
  p_value->value.string = strdup(val);
  return p_value;
}

JsonValue* json_array(void) {
  JsonValue* p_value = malloc(sizeof(JsonValue));
  p_value->type = JSON_ARRAY;
  p_value->value.array.items = NULL;
  p_value->value.array.count = 0;
  p_value->value.array.capacity = 0;
  return p_value;
}

JsonValue* json_object(void) {
  JsonValue* p_value = malloc(sizeof(JsonValue));
  p_value->type = JSON_OBJECT;
  p_value->value.object.members = NULL;
  p_value->value.object.count = 0;
  p_value->value.object.capacity = 0;
  return p_value;
}


void json_array_append(JsonValue* p_array, JsonValue* p_element) {
  if (!p_array || p_array->type != JSON_ARRAY) return;

  if (p_array->value.array.count >= p_array->value.array.capacity) {
    size_t new_cap = p_array->value.array.capacity == 0 
      ? 4 
      : p_array->value.array.capacity * 2;

    JsonValue** new_items = realloc(
      p_array->value.array.items, 
      new_cap * sizeof(JsonValue*));

    if (!new_items) return;

    p_array->value.array.items = new_items;
    p_array->value.array.capacity = new_cap;
  }

  p_array->value.array.items[p_array->value.array.count++] = p_element;
}

void json_array_set(JsonValue* p_array, size_t index, JsonValue* p_element) {
  if (
    !p_array ||
    p_array->type != JSON_ARRAY || 
    index >= p_array->value.array.count ||
    !p_element
  ) return;

  json_free(p_array->value.array.items[index]);
  p_array->value.array.items[index] = p_element;
}

JsonValue* json_array_get(const JsonValue* p_array, size_t index) {
  if (
    !p_array ||
    p_array->type != JSON_ARRAY ||
    index >= p_array->value.array.count
  ) return NULL;

  return p_array->value.array.items[index];  
}

size_t json_array_size(const JsonValue* p_array) {
  if (!p_array || p_array->type != JSON_ARRAY) return 0;

  return p_array->value.array.count;
}


void json_object_set(JsonValue* p_object, const char* key, JsonValue* p_value) {
  if (!p_object || p_object->type != JSON_OBJECT || !key || !p_value) return;

  int i = json_object_find(p_object, key);
  if (i != -1) {
    json_free(p_object->value.object.members[i].value);
    p_object->value.object.members[i].value = p_value;
  } else {
    if (p_object->value.object.count >= p_object->value.object.capacity) {
      size_t new_cap = p_object->value.object.capacity == 0
        ? 4
        : p_object->value.object.capacity * 2;
      
      JsonMember* new_members = realloc(
        p_object->value.object.members,
        new_cap * sizeof(JsonMember));
      
      if (!new_members) return;

      p_object->value.object.members = new_members;
      p_object->value.object.capacity = new_cap;
    }

    JsonMember new_member = {strdup(key), p_value};
    p_object->value.object.members[p_object->value.object.count++] = new_member;
  }
}

JsonValue* json_object_get(const JsonValue* p_object, const char* key) {
  if (!p_object || p_object->type != JSON_OBJECT || !key) return NULL;

  int match = json_object_find(p_object, key);

  if (match < 0) return NULL;

  return p_object->value.object.members[match].value;
}

void json_object_remove(JsonValue* p_object, const char* key) {
  if (!p_object || p_object->type != JSON_OBJECT || !key) return;

  int match = json_object_find(p_object, key);

  if (match == -1) return;

  JsonMember* members = p_object->value.object.members;

  free(members[match].key);
  json_free(members[match].value);

  for (size_t i = match, n = p_object->value.object.count - 1; i < n; i++){
    members[i] = members[i + 1];
  }

  p_object->value.object.count--;
}

int json_object_find(const JsonValue* p_object, const char* key) {
  if (!p_object || p_object->type != JSON_OBJECT || !key) return -1;

  for (size_t i = 0, n = p_object->value.object.count; i < n; i++) {
    if (strcmp(p_object->value.object.members[i].key, key) == 0) {
      return i;
    }
  }

  return -1;
}

size_t json_object_size(const JsonValue* p_object) {
  if (!p_object || p_object->type != JSON_OBJECT) return 0;

  return p_object->value.object.count;
}

JsonMember* json_object_iterate(JsonValue* p_object, size_t* p_index) {
  if (!p_object || p_object->type != JSON_OBJECT || !p_index) return NULL;

  if (*p_index >= p_object->value.object.count) return NULL;

  return &p_object->value.object.members[(*p_index)++];
}


void json_free(JsonValue* p_value) {
  if (!p_value) return;
  
  switch (p_value->type) {
    case JSON_STRING:
      free(p_value->value.string);
      break;
    case JSON_ARRAY:
      for (size_t i = 0, c = p_value->value.array.count; i < c; i++) {
        json_free(p_value->value.array.items[i]);
      }
      free(p_value->value.array.items);
      break;
    case JSON_OBJECT:
      for (size_t i = 0, c = p_value->value.object.count; i < c; i++) {
        free(p_value->value.object.members[i].key);
        json_free(p_value->value.object.members[i].value);
      }
      free(p_value->value.object.members);
      break;
    default:
      break;
  }

  free(p_value);
}


JsonType json_type(const JsonValue* p_value) {
  if (!p_value) return JSON_INVALID;

  return p_value->type;
}


static void strappend(char** p_buf, size_t* p_len, size_t* p_cap, const char* s) {
  size_t add = strlen(s);

  if (*p_len + add + 1 > *p_cap) {
    size_t new_cap = (*p_cap == 0) ? 64 : *p_cap;

    while (new_cap < *p_len + add + 1) {
      new_cap *= 2;
    }

    char* new_buf = realloc(*p_buf, new_cap);
    if (!new_buf) return;

    *p_buf = new_buf;
    *p_cap = new_cap;
  }

  memcpy(*p_buf + *p_len, s, add);
  *p_len += add;
  (*p_buf)[*p_len] = '\0';
}

static void indent(char** p_buf, size_t* p_len, size_t* p_cap, int depth) {
  for (int i = 0; i < depth; i++) {
    strappend(p_buf, p_len, p_cap, "  ");
  }
}

static void json_stringify_recursive(JsonValue* p_value, char** p_buf, size_t* p_len, size_t* p_cap, bool pretty, int depth) {
  char tmp[64];

  switch (p_value->type) {
    case JSON_NULL:
      strappend(p_buf, p_len, p_cap, "null");
      break;

    case JSON_BOOLEAN:
      strappend(p_buf, p_len, p_cap, p_value->value.boolean ? "true" : "false");
      break;

    case JSON_NUMBER:
      snprintf(tmp, sizeof(tmp), "%g", p_value->value.number);
      strappend(p_buf, p_len, p_cap, tmp);
      break;

    case JSON_STRING:
      strappend(p_buf, p_len, p_cap, "\"");
      strappend(p_buf, p_len, p_cap, p_value->value.string);
      strappend(p_buf, p_len, p_cap, "\"");
      break;

    case JSON_ARRAY:
      if (p_value->value.array.count == 0) {
        strappend(p_buf, p_len, p_cap, "[]");
        break;
      }

      if (pretty) strappend(p_buf, p_len, p_cap, "[\n");
      else strappend(p_buf, p_len, p_cap, "[");

      for (size_t i = 0; i < p_value->value.array.count; i++) {
        if (i > 0) {
          strappend(p_buf, p_len, p_cap, pretty ? ",\n" : ",");
        }

        if (pretty) indent(p_buf, p_len, p_cap, depth + 1);

        json_stringify_recursive(
          p_value->value.array.items[i],
          p_buf,
          p_len,
          p_cap,
          pretty,
          depth + 1
        );
      }

      if (pretty) {
        strappend(p_buf, p_len, p_cap, "\n");
        indent(p_buf, p_len, p_cap, depth);
      }

      strappend(p_buf, p_len, p_cap, "]");
      break;

    case JSON_OBJECT:
      if (p_value->value.object.count == 0) {
        strappend(p_buf, p_len, p_cap, "{}");
        break;
      }

      if (pretty) strappend(p_buf, p_len, p_cap, "{\n");
      else strappend(p_buf, p_len, p_cap, "{");

      for (size_t i = 0; i < p_value->value.object.count; i++) {
        if (i > 0) {
          strappend(p_buf, p_len, p_cap, pretty ? ",\n" : ",");
        }

        if (pretty) indent(p_buf, p_len, p_cap, depth + 1);

        strappend(p_buf, p_len, p_cap, "\"");
        strappend(p_buf, p_len, p_cap, p_value->value.object.members[i].key);
        strappend(p_buf, p_len, p_cap, "\":");

        if (pretty) strappend(p_buf, p_len, p_cap, " ");

        json_stringify_recursive(
          p_value->value.object.members[i].value,
          p_buf,
          p_len,
          p_cap,
          pretty,
          depth + 1
        );
      }

      if (pretty) {
        strappend(p_buf, p_len, p_cap, "\n");
        indent(p_buf, p_len, p_cap, depth);
      }

      strappend(p_buf, p_len, p_cap, "}");
      break;
  }
}

char* json_stringify(JsonValue* p_value, bool pretty) {
  char* buf = NULL;
  size_t len = 0;
  size_t cap = 0;

  strappend(&buf, &len, &cap, "");

  json_stringify_recursive(p_value, &buf, &len, &cap, pretty, 0);

  return buf;
}


static JsonValue* parse_value(Cursor* p_c) {
  Token* tk = peek(p_c);
  if (!tk) return NULL;

  TokenClass tc = classify(tk);

  int rule = table[NT_ELEMENT][tc];
  
  switch (rule) {
    case 2: return parse_object(p_c);
    case 3: return parse_array(p_c);
    case 4: {
      Token* t = advance(p_c);
      return json_string(t->p_value);
    }
    case 5: {
      Token* t = advance(p_c);
      return json_number(atof(t->p_value));
    }
    case 6: {
      Token* t = advance(p_c);

      if (strcmp(t->p_value, "true") == 0)
        return json_boolean(true);
      
      if (strcmp(t->p_value, "false") == 0)
        return json_boolean(false);

      return json_null();
    }

    default:
      return NULL;
  }
}

static JsonValue* parse_array(Cursor* p_c) {
  if (!match(p_c, PUNCTUATOR) || previous(p_c)->p_value[0] != '[')
    return NULL;

  JsonValue* arr = json_array();

  if (peek(p_c) &&
      peek(p_c)->type == PUNCTUATOR &&
      peek(p_c)->p_value[0] == ']') {
    advance(p_c);
    return arr;
  }

  while (1) {
    JsonValue* value = parse_value(p_c);
    if (!value) {
      json_free(arr);
      return NULL;
    }

    json_array_append(arr, value);

    if (peek(p_c) &&
        peek(p_c)->type == PUNCTUATOR &&
        peek(p_c)->p_value[0] == ',') {
      advance(p_c);
      continue;
    }

    break;
  }

  if (!match(p_c, PUNCTUATOR) || previous(p_c)->p_value[0] != ']') {
    json_free(arr);
    return NULL;
  }

  return arr;
}

static JsonValue* parse_object(Cursor* p_c) {
  if (!match(p_c, PUNCTUATOR) || previous(p_c)->p_value[0] != '{')
    return NULL;

  JsonValue* obj = json_object();

  if (peek(p_c) && peek(p_c)->type == PUNCTUATOR &&
      peek(p_c)->p_value[0] == '}') {
    advance(p_c);
    return obj;
  }

  while (1) {
    Token* key = expect(p_c, STRING);
    if (!key) {
      json_free(obj);
      return NULL;
    }

    if (!match(p_c, PUNCTUATOR) || previous(p_c)->p_value[0] != ':') {
      json_free(obj);
      return NULL;
    }

    JsonValue* value = parse_value(p_c);
    if (!value) {
      json_free(obj);
      return NULL;
    }

    json_object_set(obj, key->p_value, value);

    if (peek(p_c) &&
        peek(p_c)->type == PUNCTUATOR &&
        peek(p_c)->p_value[0] == ',') {
      advance(p_c);
      continue;
    }

    break;
  }

  if (!match(p_c, PUNCTUATOR) || previous(p_c)->p_value[0] != '}') {
    json_free(obj);
    return NULL;
  }

  return obj;
}

JsonValue* json_parse(const char* str) {
  if (!str) return NULL;

  size_t count = 0;
  char tokenizer_error[ERROR_BUFFER_SIZE] = {0};

  Token** p_tokens = tokenize(str, strlen(str), &count, tokenizer_error, ERROR_BUFFER_SIZE);

  if (!p_tokens) {
    if (json_debug_enabled) fprintf(stderr, "%s", tokenizer_error);
    return NULL;
  }

  Cursor cursor;
  init(&cursor, p_tokens, count);
  cursor.error[0] = '\0';

  init_parsing_table(table);

  JsonValue* root = parse_value(&cursor);
  
  free_tokens(p_tokens, count);
  
  if (!root) {
    if (json_debug_enabled) {
      if (cursor.error[0]) fprintf(stderr, "%s", cursor.error);
      else fprintf(stderr, "[PARSER] Unknown error\n");
    }
    return NULL;
  }

  return root;
}

JsonValue* json_parse_from_file(const char* path) {
  FILE* file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "File %s doesn't exist or don't have permission", path);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(size + 1);
  if (!buffer) {
    fprintf(stderr, "Error allocating read buffer");
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, size, file);
  buffer[size] = '\0';

  fclose(file);

  JsonValue* result = json_parse(buffer);

  free(buffer);

  return result;
}