#include "terrain.hpp"

Terrain::Terrain(Shader shadowShader) {
  model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
  model.materials[0].shader = shadowShader;
}

void Terrain::draw() {
  DrawModelEx(model, (Vector3){0.0f, -0.5f, 0.0f}, Vector3Zero(), 0.0f,
              (Vector3){80.0f, 1.0f, 80.0f}, (Color){50, 168, 82, 255});
}
