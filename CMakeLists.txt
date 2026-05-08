cmake_minimum_required(VERSION 3.10)
project(jsonlib C)

set(CMAKE_C_STANDARD 11)

# Public include directory (ONLY json.h lives here)
include_directories(include)

# Internal includes for build only
include_directories(src)

# Source files
set(SOURCES
  src/json.c
  src/tokenizer.c
  src/cursor.c
  src/table.c
)

# -------------------------
# STATIC LIBRARY
# -------------------------
add_library(json_static STATIC ${SOURCES})
set_target_properties(json_static PROPERTIES OUTPUT_NAME "json")

# -------------------------
# SHARED LIBRARY (DLL)
# -------------------------
add_library(json_shared SHARED ${SOURCES})
set_target_properties(json_shared PROPERTIES OUTPUT_NAME "json")

# Windows: export symbols automatically
target_compile_definitions(json_shared PRIVATE JSON_BUILD_DLL)
