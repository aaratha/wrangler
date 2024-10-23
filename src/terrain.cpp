#include "terrain.hpp"

Blade::Blade(Shader shadowShader, vec3 pos) : pos(pos) {
  model = LoadModel("resources/models/grass_blade.glb");
  if (model.meshCount == 0) {
    std::cerr << "Failed to load grass model!" << std::endl;
  }
  model.materials[0].shader = shadowShader;
}

void Blade::draw() {
  DrawModelEx(model, pos, (Vector3){1.0f, 0.0f, 0.0f}, 180.0f, make_vec3(1.5),
              (Color){60, 178, 92, 255});
}

Terrain::Terrain(Shader shadowShader, int bladeCount) {
  planeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
  planeModel.materials[0].shader = shadowShader;
  for (int i = 0; i < bladeCount; i++) {
    grass.push_back(
        Blade(shadowShader, (vec3){(float)GetRandomValue(-10, 10), 0.0f,
                                   (float)GetRandomValue(-10, 10)}));
  }
}

void Terrain::draw() {
  DrawModelEx(planeModel, (Vector3){0.0f, -0.5f, 0.0f}, Vector3Zero(), 0.0f,
              (Vector3){80.0f, 1.0f, 80.0f}, (Color){50, 168, 82, 255});
  for (auto blade : grass) {
    blade.draw();
  }
}
