#include "terrain.hpp"

Blade::Blade(Shader shadowShader, vec3 pos) : pos(pos) {
  model = LoadModel("resources/models/grass_blade.glb");
  if (model.meshCount == 0) {
    std::cerr << "Failed to load grass model!" << std::endl;
  }
  // model.materials[0].shader = shadowShader;
}

Terrain::Terrain(Shader shadowShader, int bladeCount)
    : blade(shadowShader, make_vec3(0.0f)), bladeCount(bladeCount) {

  Shader instancedShader = LoadShader("resources/shaders/grass.vs", "resources/shaders/grass.fs");
  instancedShader.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocationAttrib(instancedShader, "instanceTransform");
  // Assuming shader is your loaded shader program

  Material matInstances = LoadMaterialDefault();
  matInstances.shader = instancedShader;

  planeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
  planeModel.materials[0].shader = shadowShader;
  blade.model.materials[0] = matInstances;

  // Allocate memory for transformations
  transforms = (Matrix *)RL_CALLOC(bladeCount, sizeof(Matrix));

  for (int i = 0; i < bladeCount; i++) {
    Vector3 position = (Vector3){GetRandomFloat(-20, 20), 0.0f, GetRandomFloat(-20, 20)};
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);

    Vector3 axis = Vector3Normalize((Vector3){1.0, 0.0, 0.0});
    float angle = GetRandomFloat(100, 180) * DEG2RAD;
    Matrix rotation = MatrixRotate(axis, angle);

    transforms[i] = MatrixMultiply(rotation, translation);
  }
}

void Terrain::draw() {
  // Draw the terrain (plane)
  DrawModelEx(planeModel, (Vector3){0.0f, -0.5f, 0.0f}, Vector3Zero(), 0.0f,
              (Vector3){80.0f, 1.0f, 80.0f}, (Color){51, 102, 51, 255});

  // Then try instancing
  DrawMeshInstanced(blade.model.meshes[0], blade.model.materials[0], transforms, bladeCount);
}
