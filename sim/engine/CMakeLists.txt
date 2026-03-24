add_library(loongrisc_engine executor.cc)
target_include_directories(loongrisc_engine PUBLIC ${CMAKE_SOURCE_DIR}/sim)
target_link_libraries(loongrisc_engine PUBLIC loongrisc_core loongrisc_isa)
