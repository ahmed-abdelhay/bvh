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

TEST_CASE("Inside/outside classification") {
  const std::string fn = DATA_DIR + std::string("/sphere.stl");
  TriangleMesh mesh;
  bool success = ReadSTL(fn, mesh);
  REQUIRE(success);
  const BBox box = ComputeBBox(mesh.vertices);
  constexpr size_t test_count = 10000;
  const std::vector<Vec3> test_points = GenerateRandomPoints(box, test_count);
  // WriteXYZ(test_points, DATA_DIR + std::string("/sphere_test_points.xyz"));
  REQUIRE(test_points.size() == test_count);

  const std::vector<uint8_t> result0 = TestInsideOutside(mesh, test_points);
  const std::vector<uint8_t> result1 = TestInsideOutsideUsingBVH(mesh, test_points);

  //BENCHMARK("Benchmark for Brute force of in/out classification")
  //{
  //    const std::vector<uint8_t> result = TestInsideOutside(mesh, test_points);
  //};
  //BENCHMARK("Benchmark for BVH of in/out classification")
  //{
  //    const std::vector<uint8_t> result = TestInsideOutsideUsingBVH(mesh, test_points);
  //};

  REQUIRE(result0.size() == test_count);
  REQUIRE(result1.size() == test_count);

  {
    std::vector<Vec3> inside_points;
    for (size_t i = 0; i < test_count; i++) {
      if (result0[i]) {
        inside_points.push_back(test_points[i]);
      }
    }
    WriteXYZ(inside_points,
             DATA_DIR + std::string("/inside_sphere_test_points.xyz"));
  }
}