set (SRC_FILES
  include/bvh/bvh.hpp
  src/bvh.cpp)

find_package(OpenMP)

add_library(bvh STATIC ${SRC_FILES})
target_include_directories(bvh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(bvh PRIVATE OpenMP::OpenMP_CXX)

# Define the test executable
add_executable(test_bvh tests/test_bvh.cpp)
target_link_libraries(test_bvh PRIVATE bvh catch2)
target_compile_definitions(test_bvh PRIVATE DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
