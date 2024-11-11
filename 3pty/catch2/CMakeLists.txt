add_library(catch2 STATIC catch_amalgamated.cpp)
target_include_directories(catch2 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
