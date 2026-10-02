add_executable(tests_tp1 tests.cpp)
target_link_libraries(tests_tp1 PRIVATE lexique)

add_test(NAME tests_lexique
         COMMAND tests_tp1 ${CMAKE_CURRENT_SOURCE_DIR})
