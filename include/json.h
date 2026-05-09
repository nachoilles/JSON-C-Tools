#ifndef JSON
#define JSON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


static int json_debug_enabled = 0;

typedef enum {
  JSON_NULL,
  JSON_BOOLEAN,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT,
  JSON_INVALID,
} JsonType;

typedef struct JsonValue JsonValue;

typedef struct {
  char* key;
  JsonValue* value;
} JsonMember;

typedef struct JsonValue {
  JsonType type;
  union {
    bool boolean;
    double number;
    char* string;
    struct {
      JsonValue** items;
      size_t count;
      size_t capacity;
    } array;
    struct {
      JsonMember* members;
      size_t count;
      size_t capacity;
    } object;
  } value;
} JsonValue;

void json_set_debug(int enabled);

char* get_json_type_name(JsonType type);

JsonValue* json_null(void);
JsonValue* json_boolean(bool value);
JsonValue* json_number(double value);
JsonValue* json_string(const char* value);
JsonValue* json_array(void);
JsonValue* json_object(void);

void json_array_append(JsonValue* p_array, JsonValue* p_element);
void json_array_set(JsonValue* p_array, size_t index, JsonValue* p_element);
JsonValue* json_array_get(const JsonValue* p_array, size_t index);
size_t json_array_size(const JsonValue* p_array);

void json_object_set(JsonValue* p_object, const char* key, JsonValue* p_value);
JsonValue* json_object_get(const JsonValue* p_object, const char* key);
void json_object_remove(JsonValue* p_object, const char* key);
int json_object_find(const JsonValue* p_object, const char* key);
size_t json_object_size(const JsonValue* p_object);
JsonMember* json_object_iterate(JsonValue* p_object, size_t* p_index);

void json_free(JsonValue* p_value);

JsonType json_type(const JsonValue* p_value);

static void strappend(char** p_buf, size_t* p_len, size_t* p_cap, const char* s);
static void indent(char** p_buf, size_t* p_len, size_t* p_cap, int depth);
static void json_stringify_recursive(JsonValue* p_value, char** p_buf, size_t* p_len, size_t* p_cap, bool pretty, int depth);
char* json_stringify(JsonValue* p_value, bool pretty);

// Forward declaration. Implemented in cursor.h
typedef struct Cursor Cursor;

static JsonValue* parse_value(Cursor* p_c);
static JsonValue* parse_array(Cursor* p_c);
static JsonValue* parse_object(Cursor* p_c);
JsonValue* json_parse(const char* str);

#endif //JSON