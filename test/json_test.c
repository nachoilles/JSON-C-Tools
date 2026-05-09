#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "..\include\json.h"


#define ERROR_SIZE 4096

#define SUCCESS(test) printf("[%s] \033[38;5;156msuccess\x1b[37m\n", test)

#define ASSERT(cond, msg)                                                   \
    do { if (!(cond)) {                                                     \
      fprintf(stderr, "\033[31m%s\x1b[37m\n", msg);                         \
      exit(1);                                                              \
    } } while (0)

#define ASSERT_JSON_TYPE(obj, expected_type)                                \
  do {                                                                      \
    snprintf(msg, sizeof(msg),                                              \
      "[%s] expected %s but got %s",                                        \
      TEST_NAME,                                                            \
      get_json_type_name(expected_type),                                    \
      get_json_type_name((obj)->type));                                     \
    ASSERT((obj)->type == (expected_type), msg);                            \
  } while (0)

#define ASSERT_TRUE(cond, fmt, ...)                                         \
  do {                                                                      \
    snprintf(msg, sizeof(msg),                                              \
      "[%s] " fmt, TEST_NAME, ##__VA_ARGS__);                               \
    ASSERT(cond, msg);                                                      \
  } while (0)

#define ASSERT_NUMBER_EQ(actual, expected)                                  \
  ASSERT_TRUE((actual) == (expected),                                       \
    "expected %f but got %f",                                               \
    (double)(expected),                                                     \
    (double)(actual))

#define ASSERT_STRING_EQ(actual, expected)                                  \
  ASSERT_TRUE(strcmp((actual), (expected)) == 0,                            \
    "expected \"%s\" but got \"%s\"",                                       \
    (expected),                                                             \
    (actual))

#define ASSERT_BOOL_EQ(actual, expected)                                    \
  ASSERT_TRUE((actual) == (expected),                                       \
    "expected boolean %s",                                                  \
    (expected) ? "true" : "false")

#define ASSERT_SIZE_EQ(actual, expected)                                    \
  ASSERT_TRUE((actual) == (expected),                                       \
    "expected size %zu but got %zu",                                        \
    (size_t)(expected),                                                     \
    (size_t)(actual))

#define FREE_ALL(...)                                                       \
  do {                                                                      \
    JsonValue* values[] = { __VA_ARGS__ };                                  \
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++)           \
      json_free(values[i]);                                                 \
  } while (0)

static void test_create_primitives() {
  #define TEST_NAME "test_create_primitives"

  JsonValue* null_obj = json_null();
  JsonValue* bool_obj = json_boolean(true);
  JsonValue* num_obj  = json_number(123.456);
  JsonValue* str_obj  = json_string("Test");
  JsonValue* arr_obj  = json_array();
  JsonValue* obj_obj  = json_object();

  char msg[256];

  /* type checks */
  ASSERT_JSON_TYPE(null_obj, JSON_NULL);
  ASSERT_JSON_TYPE(bool_obj, JSON_BOOLEAN);
  ASSERT_JSON_TYPE(num_obj,  JSON_NUMBER);
  ASSERT_JSON_TYPE(str_obj,  JSON_STRING);
  ASSERT_JSON_TYPE(arr_obj,  JSON_ARRAY);
  ASSERT_JSON_TYPE(obj_obj,  JSON_OBJECT);

  /* value checks */
  ASSERT_BOOL_EQ(bool_obj->value.boolean, true);
  ASSERT_NUMBER_EQ(num_obj->value.number, 123.456);
  ASSERT_STRING_EQ(str_obj->value.string, "Test");
  ASSERT_SIZE_EQ(arr_obj->value.array.count, 0);
  ASSERT_SIZE_EQ(obj_obj->value.object.count, 0);

  /* cleanup */
  FREE_ALL(
    null_obj,
    bool_obj,
    num_obj,
    str_obj,
    arr_obj,
    obj_obj
  );

  SUCCESS(TEST_NAME);

  #undef TEST_NAME
}

static void test_array() {
  #define TEST_NAME "test_array"

  char msg[256];

  JsonValue* arr = json_array();

  ASSERT_JSON_TYPE(arr, JSON_ARRAY);
  ASSERT_SIZE_EQ(json_array_size(arr), 0);

  /* append elements */
  json_array_append(arr, json_number(10));
  json_array_append(arr, json_string("hello"));
  json_array_append(arr, json_boolean(true));

  ASSERT_SIZE_EQ(json_array_size(arr), 3);

  /* validate first element */
  JsonValue* first = json_array_get(arr, 0);

  ASSERT_TRUE(first != NULL,
    "expected first element to not be NULL");

  ASSERT_JSON_TYPE(first, JSON_NUMBER);
  ASSERT_NUMBER_EQ(first->value.number, 10);

  /* validate second element */
  JsonValue* second = json_array_get(arr, 1);

  ASSERT_TRUE(second != NULL,
    "expected second element to not be NULL");

  ASSERT_JSON_TYPE(second, JSON_STRING);
  ASSERT_STRING_EQ(second->value.string, "hello");

  /* validate third element */
  JsonValue* third = json_array_get(arr, 2);

  ASSERT_TRUE(third != NULL,
    "expected third element to not be NULL");

  ASSERT_JSON_TYPE(third, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(third->value.boolean, true);

  /* set element */
  json_array_set(arr, 1, json_string("world"));

  JsonValue* updated = json_array_get(arr, 1);

  ASSERT_TRUE(updated != NULL,
    "expected updated element to not be NULL");

  ASSERT_JSON_TYPE(updated, JSON_STRING);
  ASSERT_STRING_EQ(updated->value.string, "world");

  /* invalid get */
  JsonValue* invalid = json_array_get(arr, 99);

  ASSERT_TRUE(invalid == NULL,
    "expected out-of-bounds access to return NULL");

  /* invalid set should not modify array */
  json_array_set(arr, 99, json_number(999));

  ASSERT_SIZE_EQ(json_array_size(arr), 3);

  /* stress append to test resizing */
  for (int i = 0; i < 32; i++) {
    json_array_append(arr, json_number(i));
  }

  ASSERT_SIZE_EQ(json_array_size(arr), 35);

  JsonValue* last = json_array_get(arr, 34);

  ASSERT_TRUE(last != NULL,
    "expected last element to not be NULL");

  ASSERT_JSON_TYPE(last, JSON_NUMBER);
  ASSERT_NUMBER_EQ(last->value.number, 31);

  json_free(arr);

  SUCCESS(TEST_NAME);

  #undef TEST_NAME
}

static void test_object() {
  #define TEST_NAME "test_object"

  char msg[256];

  JsonValue* obj = json_object();

  ASSERT_JSON_TYPE(obj, JSON_OBJECT);
  ASSERT_SIZE_EQ(json_object_size(obj), 0);

  /* insert members */
  json_object_set(obj, "name", json_string("john"));
  json_object_set(obj, "age", json_number(25));
  json_object_set(obj, "admin", json_boolean(true));

  ASSERT_SIZE_EQ(json_object_size(obj), 3);

  /* validate string member */
  JsonValue* name = json_object_get(obj, "name");

  ASSERT_TRUE(name != NULL,
    "expected member 'name' to not be NULL");

  ASSERT_JSON_TYPE(name, JSON_STRING);
  ASSERT_STRING_EQ(name->value.string, "john");

  /* validate number member */
  JsonValue* age = json_object_get(obj, "age");

  ASSERT_TRUE(age != NULL,
    "expected member 'age' to not be NULL");

  ASSERT_JSON_TYPE(age, JSON_NUMBER);
  ASSERT_NUMBER_EQ(age->value.number, 25);

  /* validate boolean member */
  JsonValue* admin = json_object_get(obj, "admin");

  ASSERT_TRUE(admin != NULL,
    "expected member 'admin' to not be NULL");

  ASSERT_JSON_TYPE(admin, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(admin->value.boolean, true);

  /* overwrite existing key */
  json_object_set(obj, "name", json_string("alice"));

  JsonValue* updated = json_object_get(obj, "name");

  ASSERT_TRUE(updated != NULL,
    "expected updated member 'name' to not be NULL");

  ASSERT_JSON_TYPE(updated, JSON_STRING);
  ASSERT_STRING_EQ(updated->value.string, "alice");

  /* size should remain unchanged after overwrite */
  ASSERT_SIZE_EQ(json_object_size(obj), 3);

  /* find existing keys */
  ASSERT_TRUE(json_object_find(obj, "name") != -1,
    "expected to find key 'name'");

  ASSERT_TRUE(json_object_find(obj, "missing") == -1,
    "expected missing key lookup to return -1");

  /* invalid get */
  JsonValue* invalid = json_object_get(obj, "missing");

  ASSERT_TRUE(invalid == NULL,
    "expected missing key lookup to return NULL");

  /* remove member */
  json_object_remove(obj, "age");

  ASSERT_SIZE_EQ(json_object_size(obj), 2);

  ASSERT_TRUE(json_object_get(obj, "age") == NULL,
    "expected removed key 'age' to return NULL");

  /* removing missing key should do nothing */
  json_object_remove(obj, "does_not_exist");

  ASSERT_SIZE_EQ(json_object_size(obj), 2);

  /* stress insertions to test resizing */
  for (int i = 0; i < 32; i++) {
    char key[32];

    snprintf(key, sizeof(key), "key_%d", i);

    json_object_set(obj, key, json_number(i));
  }

  ASSERT_SIZE_EQ(json_object_size(obj), 34);

  JsonValue* last = json_object_get(obj, "key_31");

  ASSERT_TRUE(last != NULL,
    "expected key_31 to exist");

  ASSERT_JSON_TYPE(last, JSON_NUMBER);
  ASSERT_NUMBER_EQ(last->value.number, 31);

  /* iteration test */
  size_t index = 0;
  size_t iterated = 0;

  JsonMember* member;

  while ((member = json_object_iterate(obj, &index)) != NULL) {
    ASSERT_TRUE(member->key != NULL,
      "expected iterated member key to not be NULL");

    ASSERT_TRUE(member->value != NULL,
      "expected iterated member value to not be NULL");

    iterated++;
  }

  ASSERT_SIZE_EQ(iterated, json_object_size(obj));

  json_free(obj);

  SUCCESS(TEST_NAME);

  #undef TEST_NAME
}

static void test_stringify() {
  #define TEST_NAME "test_stringify"

  char msg[256];

  /* null */
  char* null_str = json_stringify(json_null(), false);

  ASSERT_STRING_EQ(null_str, "null");

  free(null_str);

  /* boolean */
  JsonValue* bool_val = json_boolean(true);

  char* bool_str = json_stringify(bool_val, false);

  ASSERT_STRING_EQ(bool_str, "true");

  free(bool_str);
  json_free(bool_val);

  /* number */
  JsonValue* num_val = json_number(123.456);

  char* num_str = json_stringify(num_val, false);

  ASSERT_STRING_EQ(num_str, "123.456");

  free(num_str);
  json_free(num_val);

  /* string */
  JsonValue* str_val = json_string("hello");

  char* str_str = json_stringify(str_val, false);

  ASSERT_STRING_EQ(str_str, "\"hello\"");

  free(str_str);
  json_free(str_val);

  /* empty array */
  JsonValue* empty_arr = json_array();

  char* empty_arr_str = json_stringify(empty_arr, false);

  ASSERT_STRING_EQ(empty_arr_str, "[]");

  free(empty_arr_str);
  json_free(empty_arr);

  /* populated array */
  JsonValue* arr = json_array();

  json_array_append(arr, json_number(1));
  json_array_append(arr, json_boolean(false));
  json_array_append(arr, json_string("test"));

  char* arr_str = json_stringify(arr, false);

  ASSERT_STRING_EQ(arr_str, "[1,false,\"test\"]");

  free(arr_str);
  json_free(arr);

  /* empty object */
  JsonValue* empty_obj = json_object();

  char* empty_obj_str = json_stringify(empty_obj, false);

  ASSERT_STRING_EQ(empty_obj_str, "{}");

  free(empty_obj_str);
  json_free(empty_obj);

  /* populated object */
  JsonValue* obj = json_object();

  json_object_set(obj, "name", json_string("john"));
  json_object_set(obj, "age", json_number(30));
  json_object_set(obj, "admin", json_boolean(true));

  char* obj_str = json_stringify(obj, false);

  ASSERT_STRING_EQ(
    obj_str,
    "{\"name\":\"john\",\"age\":30,\"admin\":true}"
  );

  free(obj_str);

  /* pretty stringify */
  char* pretty_str = json_stringify(obj, true);

  ASSERT_TRUE(pretty_str != NULL,
    "expected pretty stringify result to not be NULL");

  ASSERT_TRUE(strstr(pretty_str, "\n") != NULL,
    "expected pretty stringify output to contain newlines");

  ASSERT_TRUE(strstr(pretty_str, "  ") != NULL,
    "expected pretty stringify output to contain indentation");

  free(pretty_str);

  /* nested structures */
  JsonValue* nested = json_object();

  JsonValue* nested_arr = json_array();

  json_array_append(nested_arr, json_string("a"));
  json_array_append(nested_arr, json_string("b"));

  json_object_set(nested, "items", nested_arr);

  char* nested_str = json_stringify(nested, false);

  ASSERT_STRING_EQ(
    nested_str,
    "{\"items\":[\"a\",\"b\"]}"
  );

  free(nested_str);

  json_free(nested);
  json_free(obj);

  SUCCESS(TEST_NAME);

  #undef TEST_NAME
}

static void test_parse() {
  #define TEST_NAME "test_parse"

  char msg[256];

  /* null */
  JsonValue* null_val = json_parse("null");

  ASSERT_TRUE(null_val != NULL,
    "expected parsed null to not be NULL");

  ASSERT_JSON_TYPE(null_val, JSON_NULL);

  json_free(null_val);

  /* boolean true */
  JsonValue* bool_true = json_parse("true");

  ASSERT_TRUE(bool_true != NULL,
    "expected parsed true to not be NULL");

  ASSERT_JSON_TYPE(bool_true, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(bool_true->value.boolean, true);

  json_free(bool_true);

  /* boolean false */
  JsonValue* bool_false = json_parse("false");

  ASSERT_TRUE(bool_false != NULL,
    "expected parsed false to not be NULL");

  ASSERT_JSON_TYPE(bool_false, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(bool_false->value.boolean, false);

  json_free(bool_false);

  /* number */
  JsonValue* num = json_parse("123.456");

  ASSERT_TRUE(num != NULL,
    "expected parsed number to not be NULL");

  ASSERT_JSON_TYPE(num, JSON_NUMBER);
  ASSERT_NUMBER_EQ(num->value.number, 123.456);

  json_free(num);

  /* string */
  JsonValue* str = json_parse("\"hello\"");

  ASSERT_TRUE(str != NULL,
    "expected parsed string to not be NULL");

  ASSERT_JSON_TYPE(str, JSON_STRING);
  ASSERT_STRING_EQ(str->value.string, "hello");

  json_free(str);

  /* empty array */
  JsonValue* empty_arr = json_parse("[]");

  ASSERT_TRUE(empty_arr != NULL,
    "expected parsed empty array to not be NULL");

  ASSERT_JSON_TYPE(empty_arr, JSON_ARRAY);
  ASSERT_SIZE_EQ(json_array_size(empty_arr), 0);

  json_free(empty_arr);

  /* populated array */
  JsonValue* arr = json_parse("[1,\"test\",true]");

  ASSERT_TRUE(arr != NULL,
    "expected parsed array to not be NULL");

  ASSERT_JSON_TYPE(arr, JSON_ARRAY);
  ASSERT_SIZE_EQ(json_array_size(arr), 3);

  JsonValue* arr0 = json_array_get(arr, 0);
  JsonValue* arr1 = json_array_get(arr, 1);
  JsonValue* arr2 = json_array_get(arr, 2);

  ASSERT_JSON_TYPE(arr0, JSON_NUMBER);
  ASSERT_NUMBER_EQ(arr0->value.number, 1);

  ASSERT_JSON_TYPE(arr1, JSON_STRING);
  ASSERT_STRING_EQ(arr1->value.string, "test");

  ASSERT_JSON_TYPE(arr2, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(arr2->value.boolean, true);

  json_free(arr);

  /* empty object */
  JsonValue* empty_obj = json_parse("{}");

  ASSERT_TRUE(empty_obj != NULL,
    "expected parsed empty object to not be NULL");

  ASSERT_JSON_TYPE(empty_obj, JSON_OBJECT);
  ASSERT_SIZE_EQ(json_object_size(empty_obj), 0);

  json_free(empty_obj);

  /* populated object */
  JsonValue* obj = json_parse(
    "{\"name\":\"john\",\"age\":30,\"admin\":true}"
  );

  ASSERT_TRUE(obj != NULL,
    "expected parsed object to not be NULL");

  ASSERT_JSON_TYPE(obj, JSON_OBJECT);
  ASSERT_SIZE_EQ(json_object_size(obj), 3);

  JsonValue* name = json_object_get(obj, "name");
  JsonValue* age = json_object_get(obj, "age");
  JsonValue* admin = json_object_get(obj, "admin");

  ASSERT_JSON_TYPE(name, JSON_STRING);
  ASSERT_STRING_EQ(name->value.string, "john");

  ASSERT_JSON_TYPE(age, JSON_NUMBER);
  ASSERT_NUMBER_EQ(age->value.number, 30);

  ASSERT_JSON_TYPE(admin, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(admin->value.boolean, true);

  json_free(obj);

  /* nested structures */
  JsonValue* nested = json_parse(
    "{\"items\":[1,2,{\"ok\":true}]}"
  );

  ASSERT_TRUE(nested != NULL,
    "expected nested object to not be NULL");

  ASSERT_JSON_TYPE(nested, JSON_OBJECT);

  JsonValue* items = json_object_get(nested, "items");

  ASSERT_TRUE(items != NULL,
    "expected 'items' field to exist");

  ASSERT_JSON_TYPE(items, JSON_ARRAY);
  ASSERT_SIZE_EQ(json_array_size(items), 3);

  JsonValue* nested_obj = json_array_get(items, 2);

  ASSERT_JSON_TYPE(nested_obj, JSON_OBJECT);

  JsonValue* ok = json_object_get(nested_obj, "ok");

  ASSERT_JSON_TYPE(ok, JSON_BOOLEAN);
  ASSERT_BOOL_EQ(ok->value.boolean, true);

  json_free(nested);

  /* invalid JSON */

  ASSERT_TRUE(json_parse(NULL) == NULL,
    "expected parsing NULL string to fail");

  ASSERT_TRUE(json_parse("[") == NULL,
    "expected invalid array parse to fail");

  ASSERT_TRUE(json_parse("{") == NULL,
    "expected invalid object parse to fail");

  ASSERT_TRUE(json_parse("{\"a\":}") == NULL,
    "expected invalid object value parse to fail");

  ASSERT_TRUE(json_parse("[1,]") == NULL,
    "expected trailing comma array parse to fail");

  ASSERT_TRUE(json_parse("{\"a\":1,}") == NULL,
    "expected trailing comma object parse to fail");

  SUCCESS(TEST_NAME);

  #undef TEST_NAME
}

static void test_parse_from_file() {
  #define TEST_NAME "test_parse_from_file"
  JsonValue* file_data = json_parse_from_file("test.json");

  char msg[256];

  ASSERT_TRUE(file_data != NULL, "unexpected NULL");

  json_free(file_data);

  SUCCESS(TEST_NAME);

  #undef TEST_NAME
}

int main() {
  printf("========== JSON TEST ==========\n\n");
  
  test_create_primitives();
  test_array();
  test_object();
  test_parse();
  test_parse_from_file();
  
  printf("\n[json_test] \033[38;5;156mALL TEST PASSED\x1b[37m\n\n");
  return 0;
}