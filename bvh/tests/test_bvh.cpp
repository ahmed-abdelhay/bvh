#include "bvh/bvh.hpp"
#include <catch_amalgamated.hpp>

using namespace fast_bvh;

TEST_CASE("Vec3 tests") {
  Vec3 a{1, 2, 3};
  Vec3 b{3, 5, 7};
  REQUIRE(Vec3{4, 7, 10} == (a + b));
  REQUIRE(Vec3{-2, -3, -4} == (a - b));
  REQUIRE(DotProduct(a, b) == (3 + 10 + 21));
  Vec3 x{1, 0, 0};
  Vec3 y{0, 1, 0};
  REQUIRE(CrossProduct(x, y) == Vec3{0, 0, 1});
  REQUIRE(Vec3{2, 4, 6} == (a * 2));
}

TEST_CASE("Import STL") {
  const std::string fn = DATA_DIR + std::string("/sphere.stl");
  TriangleMesh result;
  bool success = ReadSTL(fn, result);
  REQUIRE(success);
  REQUIRE(result.faces.size() == 1280);
  REQUIRE(result.vertices.size() == 642);
}