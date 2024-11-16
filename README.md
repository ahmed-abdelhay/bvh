# Build instructions

- For windows you need to download and install visual studio and CMake.
- For Linux you need to install CMake and GCC or Clang.

```
https://github.com/ahmed-abdelhay/bvh.git
cd bvh
mkdir build
cd build
cmake ..
cmake --build .
```

# Plan 

## Day 1

- Goals.
- Introduction to the field of geometry processing.
  - Problems.
  - Applications.
  - Perquisites.
  - Representation of 3D objects.
  - What is a mesh?
- Code setup (tools, language, libraries).
- 3D vectors.
- Implement 3d vector.
- Meshes.
  - Different formats.
  - Loading binary STL files.

## Day 2

- Ray/Triangle intersection implementation.
- Brute force implementation of inside/outside classification.
- Benchmark.
- Why we need acceleration structures?
- Initial BVH implementation.

## Day 3

- BVH construction benchmark.
- Array based implementation.
- SIMD optimisation.
- CPU parallel implementation.
- GPU parallel implementation (CUDA?).

## Day 4

- LBVH paper.
