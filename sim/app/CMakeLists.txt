add_executable(loongrisc_sim main.cc)
target_include_directories(loongrisc_sim PRIVATE ${CMAKE_SOURCE_DIR}/sim)
target_link_libraries(loongrisc_sim PRIVATE loongrisc_engine)
