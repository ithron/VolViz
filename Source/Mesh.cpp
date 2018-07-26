#include "Mesh.h"
#include "VisualizerImpl.h"

#include <Eigen/Geometry>

#include <iostream>

namespace VolViz {
namespace Private_ {

using Lock = std::lock_guard<std::mutex>;

Mesh::Mesh(MeshDescriptor const &descriptor, VisualizerImpl &visualizer)
    : Geometry(descriptor, visualizer) {
  scale = descriptor.scale;
  updateQueue_.enqueue(descriptor);
}

void Mesh::doInit() { uploadMesh(); }

void Mesh::doRender(std::uint32_t index, bool selected) {
  Length const rScale = visualizer_.cachedScale;
  auto cameraClient = visualizer_.cameraClient();
  auto &shaders = visualizer_.shaders();

  auto const viewMat = cameraClient.viewMatrix(rScale);
  auto const destScale = static_cast<float>(scale / rScale);
  // auto const volSize = visualizer_.volumeSize();

  Matrix4 const modelMat =
      (Eigen::Translation3f(position) * orientation * Eigen::Scaling(destScale))
          .matrix();
  // Matrix4 const modelMat = (Eigen::Translation3f(position * destScale) *
  //                           orientation * volSize.asDiagonal())
  //                              .matrix();
  Matrix4 const modelViewMat = (viewMat * modelMat).eval();
  Matrix3 const inverseModelViewMatrix =
      modelViewMat.block<3, 3>(0, 0).inverse();

  // Setup shader uniforms
  assertGL("Pevious OpenGL error");
  shaders["geometryStage"].use();
  visualizer_.attachVolumeToShader(shaders["geometryStage"]);
  shaders["geometryStage"]["index"] = index;
  shaders["geometryStage"]["color"] = selected ? (color * 1.2f).eval() : color;
  shaders["geometryStage"]["shininess"] = selected ? 10.0f : 1000.f;
  shaders["geometryStage"]["modelViewProjectionMatrix"] =
      (cameraClient.projectionMatrix() * modelViewMat).eval();
  shaders["geometryStage"]["inverseModelViewMatrix"] = inverseModelViewMatrix;
  shaders["geometryStage"]["textureTransformMatrix"] =
      (visualizer_.textureTransformationMatrix() * modelMat).eval();

  assertGL("Setting uniforms failed");

  auto const vaoBinding = GL::binding(vertexArrayObject_);
  glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(numTriangles_),
                 GL_UNSIGNED_INT, nullptr);
  assertGL("glDrawElements failed");
}

void Mesh::doUpdate() { uploadMesh(); }

void Mesh::uploadMesh() {
  using GL::Buffer;
  using GL::VertexArray;

  MeshDescriptor descriptor;

  if (!updateQueue_.try_dequeue(descriptor)) return;

  auto const N = descriptor.vertices.rows();
  auto const M = descriptor.indices.rows();
  auto const vertBuffSize = N * 8 * narrow_cast<int>(sizeof(float));
  auto const indexBufferSize = M * 3 * narrow_cast<int>(sizeof(std::uint32_t));

  // Create and map vertex buffer
  Buffer vertBuffer;
  auto const vertBinding =
      binding(vertBuffer, static_cast<GLenum>(GL_ARRAY_BUFFER));
  glBufferData(GL_ARRAY_BUFFER, vertBuffSize, nullptr, GL_STATIC_DRAW);
  assertGL("glBufferData failed");
  float *mappedVertBuffer =
      reinterpret_cast<float *>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

  Ensures(mappedVertBuffer != nullptr);

  Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, 8, Eigen::RowMajor>> vertices(
      mappedVertBuffer, N, 8);

  // Create and map index buffer
  Buffer indexBuffer;
  auto const indexBinding =
      binding(indexBuffer, static_cast<GLenum>(GL_ELEMENT_ARRAY_BUFFER));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, nullptr,
               GL_STATIC_DRAW);
  std::uint32_t *mappedIndexBuffer = reinterpret_cast<std::uint32_t *>(
      glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
  Ensures(mappedIndexBuffer != nullptr);
  Eigen::Map<Eigen::Matrix<std::uint32_t, Eigen::Dynamic, 3, Eigen::RowMajor>>
      indices(mappedIndexBuffer, M, 3);

  assertGL("Failed to create or map buffers");

  // copy vertices
  for (int i = 0; i < N; ++i) {
    vertices.row(i) << descriptor.vertices.row(i), 0.f, 0.f, 0.f, 0.f, 0.f;
  }

  // copy indices
  indices = descriptor.indices;

  // compute normals
  for (int i = 0; i < M; ++i) {
    auto const &I = descriptor.indices;
    auto &V = vertices;
    Vector3f const normal =
        (V.row(I(i, 1)) - V.row(I(i, 0)))
            .head<3>()
            .cross((V.row(I(i, 2)) - V.row(I(i, 0))).head<3>())
            .normalized();
    V.block<1, 3>(I(i, 0), 4) += normal;
    V.block<1, 3>(I(i, 1), 4) += normal;
    V.block<1, 3>(I(i, 2), 4) += normal;
  }

  // normalize normals
  for (int i = 0; i < N; ++i) vertices.block<1, 3>(i, 4).normalize();

  // unmap buffers
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glUnmapBuffer(GL_ARRAY_BUFFER);

  // setup VAO
  VertexArray vao;
  auto const vaoBinding = binding(vao);
  indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
  vao.enableVertexAttribArray(0);
  vao.enableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * sizeof(float), nullptr);
  glVertexAttribPointer(1, 3, GL_FLOAT, false, 8 * sizeof(float),
                        reinterpret_cast<void const *>(4 * sizeof(float)));

  vertexBuffer_ = std::move(vertBuffer);
  indexBuffer_ = std::move(indexBuffer);
  vertexArrayObject_ = std::move(vao);
  numTriangles_ = static_cast<std::size_t>(M);
}

void Mesh::doEnqueueUpdate(GeometryDescriptor const &descriptor) {
  updateQueue_.enqueue(dynamic_cast<MeshDescriptor const &>(descriptor));
}

void Mesh::doEnqueueUpdate(GeometryDescriptor &&descriptor) {
  updateQueue_.enqueue(std::move(dynamic_cast<MeshDescriptor &&>(descriptor)));
}

} // namespace Private_
} // namespace VolViz
