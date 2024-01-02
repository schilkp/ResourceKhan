cmake_minimum_required(VERSION 3.12)

project("ResourceKhan" C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(RK_USE_CUSTOM_CONF)
add_compile_options(-Wall -Wextra -Wpedantic)

# Compile resource_khan lib to static lib:
add_library(RK STATIC src/resource_khan.c)
target_include_directories(RK PUBLIC src)
target_include_directories(RK PUBLIC test)
target_link_libraries(RK PUBLIC TestFramework)

# Compile test framework + utils to static lib:
add_library(TestFramework STATIC test/Unity/unity.c test/utils.c)
target_include_directories(TestFramework PUBLIC src)
target_include_directories(TestFramework PUBLIC test)
target_include_directories(TestFramework PUBLIC test/Unity)

# Compile recursive reference implementation to static lib::
add_library(RK_Recursive_Reference STATIC test/recursive_reference_impl.c)
target_include_directories(RK_Recursive_Reference PUBLIC src)
target_include_directories(RK_Recursive_Reference PUBLIC test)
target_link_libraries(RK_Recursive_Reference PUBLIC TestFramework)

# Util function to add a new test file.
# Generates two executeable targets for each test. One linked against the actual library
# implementation, one linked against the recursive reference implementation (test/recursive_reference_impl.c).
function(add_pt_double_test TEST_SOURCE)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WLE)

    add_executable(ACTUAL_${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(ACTUAL_${TEST_NAME} PUBLIC TestFramework RK)

    add_executable(RECREF_${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(RECREF_${TEST_NAME} PUBLIC TestFramework RK_Recursive_Reference)
endfunction()

# Util function to add a new test file.
function(add_pt_test TEST_SOURCE)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WLE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PUBLIC TestFramework RK)
endfunction()


add_pt_double_test(test/test_basic.c)
add_pt_double_test(test/test_complex1.c)
add_pt_double_test(test/test_complex2.c)
add_pt_double_test(test/test_detect_loop1.c)
add_pt_double_test(test/test_detect_loop2.c)
add_pt_double_test(test/test_detect_loop3.c)
add_pt_double_test(test/test_cb_errors.c)
add_pt_double_test(test/test_cb_calls.c)

add_pt_test(test/test_catch_nullptr.c)
