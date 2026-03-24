include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(loongrisc_sim_ut instruction_ut.cc)
target_link_libraries(loongrisc_sim_ut PRIVATE loongrisc_engine GTest::gtest_main)
target_include_directories(loongrisc_sim_ut PRIVATE ${CMAKE_SOURCE_DIR}/sim)

include(GoogleTest)
gtest_discover_tests(loongrisc_sim_ut)
