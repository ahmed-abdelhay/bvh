#include "bvh/bvh.hpp"
#include <fstream>
#include <random>
#include <span>
#include <unordered_map>

namespace fast_bvh {

// https://gist.github.com/eugene-malashkin/884e225ff57aca1b9cbe

template <class T> inline void HashCombine(std::size_t &seed, const T &v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct Vec3Hash {
  size_t operator()(const Vec3 &v) const {
    size_t seed = 0;
    HashCombine(seed, v.x);
    HashCombine(seed, v.y);
    HashCombine(seed, v.z);
    return seed;
  }
};

struct Buffer {
  uint8_t *data = nullptr;
  size_t size = 0;
  size_t cursor = 0;
};

template <typename T> bool Read(Buffer &buffer, T &result) {
  constexpr size_t bytes_to_read = sizeof(T);
  if ((buffer.cursor + bytes_to_read) > buffer.size) {
    return false;
  }
  ::memcpy(&result, buffer.data + buffer.cursor, bytes_to_read);
  buffer.cursor += bytes_to_read;
  return true;
}

bool ReadFile(const char *fileName, std::vector<uint8_t> &output) {
  FILE *fp = fopen(fileName, "rb");
  if (!fp) {
    return false;
  }
  fseek(fp, 0, SEEK_END);
  const int64_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  output.resize(size);
  const size_t read = fread(output.data(), size, 1, fp);
  fclose(fp);
  return read == 1;
}

struct DeduplicateVerticesResult {
  std::vector<Vec3> unique_vertices;
  std::vector<size_t> old_to_new_map;
};

DeduplicateVerticesResult DeduplicateVertices(std::span<const Vec3> vertices) {
  DeduplicateVerticesResult result;
  result.old_to_new_map.resize(vertices.size());
  std::unordered_map<Vec3, size_t, Vec3Hash> hash_map;
  const size_t vertex_count = vertices.size();
  for (size_t i = 0; i < vertex_count; ++i) {
    const Vec3 &v = vertices[i];
    if (!hash_map.count(v)) {
      const size_t current_count = hash_map.size();
      result.unique_vertices.push_back(v);
      hash_map[v] = current_count;
    }
    result.old_to_new_map[i] = hash_map[v];
  }
  return result;
}

bool ReadSTL(const std::string &filename, TriangleMesh &result) {
  std::vector<uint8_t> file_data;
  if (!ReadFile(filename.c_str(), file_data)) {
    return false;
  }

  Buffer buffer;
  buffer.data = file_data.data();
  buffer.size = file_data.size();
  buffer.cursor = 0;

  buffer.cursor += 80;

  uint32_t faces_count = 0;
  Read(buffer, faces_count);

  /*
    REAL32[3] – Normal vector             - 12 bytes
    REAL32[3] – Vertex 1                  - 12 bytes
    REAL32[3] – Vertex 2                  - 12 bytes
    REAL32[3] – Vertex 3                  - 12 bytes
    UINT16    – Attribute byte count      -  2 bytes
  */
  result.faces.resize(faces_count);
  result.vertices.resize(3 * faces_count);
  for (size_t i = 0; i < faces_count; i++) {
    Vec3 normal;
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
    uint16_t attribute;
    if (!Read(buffer, normal) || !Read(buffer, v0) || !Read(buffer, v1) ||
        !Read(buffer, v2) || !Read(buffer, attribute)) {
      return false;
    }
    result.vertices[i * 3 + 0] = v0;
    result.vertices[i * 3 + 1] = v1;
    result.vertices[i * 3 + 2] = v2;
    result.faces[i].a = i * 3 + 0;
    result.faces[i].b = i * 3 + 1;
    result.faces[i].c = i * 3 + 2;
  }

  DeduplicateVerticesResult dedup = DeduplicateVertices(result.vertices);
  result.vertices = dedup.unique_vertices;
  for (Triangle &face : result.faces) {
    face.a = dedup.old_to_new_map[face.a];
    face.b = dedup.old_to_new_map[face.b];
    face.c = dedup.old_to_new_map[face.c];
  }
  return true;
}

void WriteXYZ(std::span<const Vec3> points, const std::string &filename) {
  std::ofstream fout(filename);
  for (const Vec3 &v : points) {
    fout << v.x << " " << v.y << " " << v.z << '\n';
  }
}

BBox ComputeBBox(std::span<const Vec3> vertices) {
  BBox result;
  for (const Vec3 &v : vertices) {
    result.Add(v);
  }
  return result;
}

std::vector<Vec3> GenerateRandomPoints(const BBox &box, size_t count) {
  std::vector<Vec3> result(count);
  constexpr int seed = 42;
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> dist_x(box.min.x, box.max.x);
  std::uniform_real_distribution<float> dist_y(box.min.y, box.max.y);
  std::uniform_real_distribution<float> dist_z(box.min.z, box.max.z);
  for (size_t i = 0; i < count; i++) {
    result[i].x = dist_x(rng);
    result[i].y = dist_y(rng);
    result[i].z = dist_z(rng);
  }
  return result;
}

std::optional<Vec3> CheckRayTriangleIntersection(const Vec3 &ta, const Vec3 &tb,
                                                 const Vec3 &tc,
                                                 const Ray &ray) {
  constexpr float epsilon = std::numeric_limits<float>::epsilon();

  Vec3 edge1 = tb - ta;
  Vec3 edge2 = tc - ta;
  Vec3 ray_cross_e2 = CrossProduct(ray.direction, edge2);
  float det = DotProduct(edge1, ray_cross_e2);

  if (det > -epsilon && det < epsilon)
    return {}; // This ray is parallel to this triangle.

  float inv_det = 1.0 / det;
  Vec3 s = ray.origin - ta;
  float u = inv_det * DotProduct(s, ray_cross_e2);

  if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u - 1) > epsilon))
    return {};

  Vec3 s_cross_e1 = CrossProduct(s, edge1);
  float v = inv_det * DotProduct(ray.direction, s_cross_e1);

  if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon))
    return {};

  // At this stage we can compute t to find out where the intersection point is
  // on the line.
  float t = inv_det * DotProduct(edge2, s_cross_e1);

  if (t > epsilon) // ray intersection
  {
    return Vec3(ray.origin + ray.direction * t);
  }
  return {}; // This means that there is a line intersection but not a ray
             // intersection.
}

std::vector<uint8_t> TestInsideOutside(const TriangleMesh &mesh,
                                       std::span<const Vec3> test_points) {
  const size_t test_count = test_points.size();
  const size_t triangle_count = mesh.faces.size();

  std::vector<uint8_t> result(test_count);

#pragma omp parallel for
  for (int64_t i = 0; i < test_count; i++) {
    const Vec3 &test_point = test_points[i];

    Ray ray;
    ray.origin = test_point;
    ray.direction = Vec3{1, 0, 0};

    size_t intersection_count = 0;
    for (size_t j = 0; j < triangle_count; j++) {
      const Triangle &t = mesh.faces[j];
      const Vec3 &ta = mesh.vertices[t.a];
      const Vec3 &tb = mesh.vertices[t.b];
      const Vec3 &tc = mesh.vertices[t.c];

      if (CheckRayTriangleIntersection(ta, tb, tc, ray)) {
        intersection_count++;
      }
    }
    result[i] = intersection_count % 2;
  }

  return result;
}

Vec3 BBoxSize(const BBox &box) { return box.max - box.min; }

Vec3 ComputeTriangleCenter(const Vec3 &a, const Vec3 &b, const Vec3 &c) {
  return (a + b + c) * (1. / 3.);
}

struct BVH {
  struct Node {
    BBox box;
    int child0 = -1;
    int child1 = -1;
    int face_id = -1;
  };

  std::vector<Node> nodes;
};

BVH BuildBVH(const TriangleMesh &mesh) {

  struct Stack {
    int parent_node = -1;
    bool is_child0 = false;
    BBox box;
    std::vector<int> face_ids;
  };
  BVH result;

  const BBox mesh_box = ComputeBBox(mesh.vertices);
  std::vector<int> face_ids(mesh.faces.size());
  for (size_t i = 0; i < mesh.faces.size(); i++) {
    face_ids[i] = i;
  }

  std::vector<Stack> stack{Stack{-1, false, mesh_box, face_ids}};
  while (!stack.empty()) {
    const Stack top = stack.back();
    stack.pop_back();

    const int node_id = result.nodes.size();
    if (top.parent_node >= 0) {
      if (top.is_child0) {
        result.nodes[top.parent_node].child0 = node_id;
      } else {
        result.nodes[top.parent_node].child1 = node_id;
      }
    }

    if (top.face_ids.size() == 1) {
      BVH::Node node;
      node.box = top.box;
      node.face_id = top.face_ids[0];
      if (top.parent_node >= 0) {
        if (top.is_child0) {
          result.nodes[top.parent_node].child0 = node_id;
        } else {
          result.nodes[top.parent_node].child1 = node_id;
        }
      }
      result.nodes.push_back(node);
    } else {
      const Vec3 box_size = BBoxSize(top.box);
      const size_t split_dir =
          (box_size.x > box_size.y && box_size.x > box_size.z)   ? 0
          : (box_size.y > box_size.x && box_size.y > box_size.z) ? 1
                                                                 : 2;
      const float split_point =
          (top.box.min[split_dir] + top.box.max[split_dir]) * 0.5;

      Stack left, right;
      for (int face_id : top.face_ids) {
        const Triangle &face = mesh.faces[face_id];
        const Vec3 triangle_center =
            ComputeTriangleCenter(mesh.vertices[face.a], mesh.vertices[face.b],
                                  mesh.vertices[face.c]);
        if (triangle_center[split_dir] < split_point) {
          left.face_ids.push_back(face_id);
        } else {
          right.face_ids.push_back(face_id);
        }
      }

      const int parent_node_id = node_id;
      BVH::Node internal_node;
      internal_node.box = top.box;
      result.nodes.push_back(internal_node);

      if (!left.face_ids.empty()) {
        left.is_child0 = true;
        left.parent_node = parent_node_id;
        for (int face_id : left.face_ids) {
          const Triangle &face = mesh.faces[face_id];
          left.box.Add(mesh.vertices[face.a]);
          left.box.Add(mesh.vertices[face.b]);
          left.box.Add(mesh.vertices[face.c]);
        }
        stack.push_back(left);
      }
      if (!right.face_ids.empty()) {
        right.is_child0 = false;
        right.parent_node = parent_node_id;
        for (int face_id : right.face_ids) {
          const Triangle &face = mesh.faces[face_id];
          right.box.Add(mesh.vertices[face.a]);
          right.box.Add(mesh.vertices[face.b]);
          right.box.Add(mesh.vertices[face.c]);
        }
        stack.push_back(right);
      }
    }
  }

  return result;
}

size_t BVHRayIntersection(const BVH &tree, Ray ray) { return 0; }

std::vector<uint8_t>
TestInsideOutsideUsingBVH(const TriangleMesh &mesh,
                          std::span<const Vec3> test_points) {
  const size_t test_count = test_points.size();
  const size_t triangle_count = mesh.faces.size();

  const BVH tree = BuildBVH(mesh);

  std::vector<uint8_t> result(test_count);

#pragma omp parallel for
  for (int64_t i = 0; i < test_count; i++) {
    const Vec3 &test_point = test_points[i];

    Ray ray;
    ray.origin = test_point;
    ray.direction = Vec3{1, 0, 0};

    const size_t intersection_count = BVHRayIntersection(tree, ray);
    result[i] = intersection_count % 2;
  }

  return result;
}

} // namespace fast_bvh
