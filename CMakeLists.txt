cmake_minimum_required(VERSION 3.20)

project(cc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options(/wd4838 /Zi)
else()
  add_compile_options(-Wno-narrowing -g)
endif()

set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
set(GENERATED_PPTOKEN_DIR ${GENERATED_DIR}/pptoken)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

add_library(utils
  utils/file_utils.cpp
  utils/string_utils.cpp
)

add_library(grammar_common
  lex/grammar_common.cpp
)

target_link_libraries(grammar_common utils)

add_executable(lex lex/lex.cpp)
target_link_libraries(lex grammar_common utils)
set_target_properties(lex PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lex
)

add_executable(gnm gnm/gnm.cpp
        gnm/ast.h
        gnm/parse.h
        gnm/parse.cpp
        gnm/pretty_print.h
        gnm/pretty_print.cpp)
target_link_libraries(gnm grammar_common utils)
set_target_properties(gnm PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/gnm
)

set(PPTOKEN_GENERATED
  ${GENERATED_PPTOKEN_DIR}/pptoken.enum.cpp
  ${GENERATED_PPTOKEN_DIR}/pptoken.regex.cpp
  ${GENERATED_PPTOKEN_DIR}/pptoken.match.cpp
  ${GENERATED_PPTOKEN_DIR}/pptoken.begin.cpp
)

add_custom_command(
  OUTPUT ${PPTOKEN_GENERATED}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_PPTOKEN_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/pptoken/pptoken.lex
    ${GENERATED_PPTOKEN_DIR}/pptoken.lex
  COMMAND $<TARGET_FILE:lex> ${GENERATED_PPTOKEN_DIR}/pptoken.lex
  DEPENDS lex ${CMAKE_CURRENT_SOURCE_DIR}/pptoken/pptoken.lex
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

add_custom_target(generate_pptoken
  DEPENDS ${PPTOKEN_GENERATED}
)

add_library(pptoken
  pptoken/pptoken.cpp
)

add_dependencies(pptoken generate_pptoken)

add_executable(pp_test
  pptoken/pp_test.cpp
)

target_link_libraries(pp_test pptoken utils)

set_target_properties(pp_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pptoken
)

add_executable(ctrlexpr_test
  ctrlexpr/test.cpp
  ctrlexpr/ctrlexpr.cpp
)

add_dependencies(ctrlexpr_test generate_pptoken gnm)

target_link_libraries(ctrlexpr_test pptoken utils)

set_target_properties(ctrlexpr_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ctrlexpr
)

enable_testing()

add_test(NAME pp_test
  COMMAND ${CMAKE_BINARY_DIR}/pptoken/pp_test
)

set_tests_properties(pp_test PROPERTIES
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_test(NAME ctrlexpr_test
  COMMAND ${CMAKE_BINARY_DIR}/ctrlexpr/ctrlexpr_test
)

set_tests_properties(ctrlexpr_test PROPERTIES
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
