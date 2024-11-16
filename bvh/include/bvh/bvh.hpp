#pragma once

#include <math.h>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace fast_bvh {

struct Vec3 {
  float x = 0, y = 0, z = 0;
};

inline Vec3 operator+(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vec3 operator-(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline float DotProduct(const Vec3 &v1, const Vec3 &v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
inline Vec3 CrossProduct(const Vec3 &a, const Vec3 b) {
  Vec3 c;
  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;
  return c;
}

inline float Length2(const Vec3 &a) {
  return (a.x * a.x + a.y * a.y + a.z * a.z);
}
inline float Length(const Vec3 &a) { return sqrt(Length2(a)); }

inline Vec3 operator*(const Vec3 &v, const float scale) {
  return Vec3{v.x * scale, v.y * scale, v.z * scale};
}

inline bool operator==(const Vec3 &a, const Vec3 &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}
inline bool operator!=(const Vec3 &a, const Vec3 &b) { return !(a == b); }

struct Triangle {
  size_t a = 0, b = 0, c = 0;
};

struct TriangleMesh {
  std::vector<Vec3> vertices;
  std::vector<Triangle> faces;
};

bool ReadSTL(const std::string &filename, TriangleMesh &result);
void WriteXYZ(std::span<const Vec3> points, const std::string &filename);

constexpr float float_max = std::numeric_limits<float>::max();

struct BBox {
  Vec3 min = Vec3{float_max, float_max, float_max};
  Vec3 max = Vec3{-float_max, -float_max, -float_max};

  void Add(const Vec3 &p) {
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    min.z = std::min(min.z, p.z);
    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
    max.z = std::max(max.z, p.z);
  }
};

struct Ray {
  Vec3 origin;
  Vec3 direction;
};

BBox ComputeBBox(std::span<const Vec3> vertices);

std::optional<Vec3> CheckRayTriangleIntersection(const Vec3 &ta, const Vec3 &tb,
                                                 const Vec3 &tc,
                                                 const Ray &ray);

std::vector<Vec3> GenerateRandomPoints(const BBox &box, size_t count);
std::vector<uint8_t> TestInsideOutside(const TriangleMesh &mesh,
                                       std::span<const Vec3> test_points);

} // namespace fast_bvh
