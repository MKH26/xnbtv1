cmake_minimum_required(VERSION 3.21)
project(xnbt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── LeviLamina SDK path (set via -DLEVILAMINADIR or env) ──────────────────────
if(NOT DEFINED LEVILAMINADIR)
    set(LEVILAMINADIR "$ENV{LEVILAMINADIR}" CACHE PATH "LeviLamina SDK root")
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE XNBT_SOURCES
    "src/*.cpp"
    "gui/*.cpp"
    "nbt/*.cpp"
    "utils/*.cpp"
)

add_library(xnbt SHARED ${XNBT_SOURCES})

target_include_directories(xnbt PRIVATE
    include
    ${LEVILAMINADIR}/include
    ${LEVILAMINADIR}/third-party/include
)

target_compile_definitions(xnbt PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    XNBT_VERSION="1.0.0"
)

target_compile_options(xnbt PRIVATE
    -O2
    -fvisibility=hidden
    -fno-exceptions         # optional – saves binary size
    -Wall
    -Wextra
)

# Link LeviLamina + Android system libs
target_link_libraries(xnbt PRIVATE
    ${LEVILAMINADIR}/lib/libLeviLamina.so
    android
    log
)

set_target_properties(xnbt PROPERTIES
    OUTPUT_NAME "xnbt"
    PREFIX      ""
    SUFFIX      ".so"
)
