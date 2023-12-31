cmake_minimum_required(VERSION 3.12)

project("Power Tree" C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_compile_definitions(PWR_TREE_USE_CUSTOM_CONF)

# Compile pwr_tree lib to static lib:
add_library(PT STATIC src/pwr_tree.c)
target_include_directories(PT PUBLIC src/)
target_include_directories(PT PUBLIC test/) # Needed for custom conf header.
target_link_libraries(PT PUBLIC Unity) # Needed for assert macros in implementation.

# Compile test framework to static lib:
add_library(Unity STATIC test/Unity/unity.c)
target_include_directories(Unity PUBLIC test/Unity)

# Compile recursive reference implementation to static lib::
add_library(PT_Recursive_Reference STATIC test/recursive_reference_impl.c)
target_include_directories(PT_Recursive_Reference PUBLIC src/)
target_include_directories(PT_Recursive_Reference PUBLIC test/) # Needed for custom conf header.
target_link_libraries(PT_Recursive_Reference PUBLIC Unity) # Needed for assert macros in implementation.

# Util function to add a new test file.
# Generates two executeable targets for each test. One linked against the actual pwr_tree
# implementation, one linked against the recursive reference implementation (test/recursive_reference_impl.c).
function(add_pt_test TEST_SOURCE)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WLE)

    add_executable(ACTUAL_${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(ACTUAL_${TEST_NAME} PUBLIC Unity PT)

    add_executable(RECREF_${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(RECREF_${TEST_NAME} PUBLIC Unity PT_Recursive_Reference)
endfunction()

add_pt_test(test/test_basic.c)
add_pt_test(test/test_complex.c)
add_pt_test(test/test_cb_errors.c)
add_pt_test(test/test_catch_errors.c)
