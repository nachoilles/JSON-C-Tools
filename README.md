# JSON C TOOLS Library

A lightweight JSON library written in C with support for parsing, stringifying, and manipulating JSON data structures.

It includes:
- JSON tokenizer and parser
- Dynamic arrays and objects
- Stringify (pretty + compact)
- Static and dynamic library builds
- Unit tests

## Build (CMake)

.\build.bat

## Tests

cd .\test\

:: Run all tests
.\run_test.bat

:: Run tokenizer test
.\run_test.bat tokenizer

:: Run cursor test
.\run_test.bat cursor

:: Run json test
.\run_test.bat json

## Usage examples

1. Parsing a JSON string

This example shows how to parse a JSON string into a JsonValue tree and access object fields using the library.

int main() {
    const char* input = "{\"name\":\"john\",\"age\":30,\"admin\":true}";

    JsonValue* value = json_parse(input);

    if (!value) {
        printf("Parse error\n");
        return 1;
    }

    JsonValue* name = json_object_get(value, "name");
    JsonValue* age  = json_object_get(value, "age");

    printf("name: %s\n", name->value.string);
    printf("age: %g\n", age->value.number);

    json_free(value);
    return 0;
}

2. Building and stringifying JSON

This example demonstrates how to manually construct a JSON object and convert it back into a JSON string.

int main() {
    JsonValue* obj = json_object();

    json_object_set(obj, "name", json_string("alice"));
    json_object_set(obj, "score", json_number(99.5));
    json_object_set(obj, "active", json_boolean(true));

    JsonValue* arr = json_array();
    json_array_append(arr, json_number(1));
    json_array_append(arr, json_number(2));
    json_array_append(arr, json_number(3));

    json_object_set(obj, "values", arr);

    char* str = json_stringify(obj, true);

    printf("%s\n", str);

    free(str);
    json_free(obj);

    return 0;
}