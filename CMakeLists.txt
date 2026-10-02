cmake_minimum_required(VERSION 3.14)
project(SEC2_TP1_Lexique LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Type de build" FORCE)
endif()

# Bibliothèque commune au programme et aux tests
add_library(lexique STATIC
    src/lexique.cpp
    src/lexique_ligne.cpp
    src/utilitaire.cpp
)
target_include_directories(lexique PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(lexique PUBLIC /W4 /utf-8)
else()
    target_compile_options(lexique PUBLIC -Wall -Wextra -Wpedantic)
endif()

# Programme de démonstration
add_executable(tp1 src/main.cpp)
target_link_libraries(tp1 PRIVATE lexique)

# Jeux d'essais
add_executable(tests_tp1 tests/tests.cpp)
target_link_libraries(tests_tp1 PRIVATE lexique)

enable_testing()
add_test(NAME tests_lexique
         COMMAND tests_tp1 ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# cmake --build build --target run : lance la démonstration sur data/
add_custom_target(run
    COMMAND tp1 ${CMAKE_CURRENT_SOURCE_DIR}/data
    DEPENDS tp1
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
)
