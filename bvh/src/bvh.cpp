#include "bvh/bvh.hpp"

namespace fast_bvh {

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
  return true;
}

} // namespace fast_bvh
