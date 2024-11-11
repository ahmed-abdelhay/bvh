#pragma once

#include <math.h>
#include <string>
#include <vector>


namespace fast_bvh {

struct Vec3 {
  float x = 0, y = 0, z = 0;
};

inline Vec3 operator+(const Vec3 &a, const Vec3 &b) { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vec3 operator-(const Vec3 &a, const Vec3 &b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }

inline float DotProduct(const Vec3 &v1, const Vec3 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
inline Vec3 CrossProduct(const Vec3 &a, const Vec3 b) {
  Vec3 c;
  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;
  return c;
}

inline float Length2(const Vec3 &a) { return (a.x * a.x + a.y * a.y + a.z * a.z); }
inline float Length(const Vec3 &a) { return sqrt(Length2(a)); }

inline Vec3 operator*(const Vec3 &v, const float scale) { return Vec3{v.x * scale, v.y * scale, v.z * scale}; }

inline bool operator==(const Vec3 &a, const Vec3 &b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
inline bool operator!=(const Vec3 &a, const Vec3 &b) { return !(a == b); }

struct Triangle {
  size_t a = 0, b = 0, c = 0;
};

struct TriangleMesh {
  std::vector<Vec3> vertices;
  std::vector<Triangle> faces;
};

bool ReadSTL(const std::string &filename, TriangleMesh &result);

} // namespace fast_bvh
