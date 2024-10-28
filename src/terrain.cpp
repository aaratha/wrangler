#include "terrain.hpp"

Blade::Blade(Shader shadowShader, vec3 pos) : pos(pos) {
  model = LoadModel("resources/models/grass_blade.glb");
  if (model.meshCount == 0) {
    std::cerr << "Failed to load grass model!" << std::endl;
  }
  // model.materials[0].shader = shadowShader;
}

Terrain::Terrain(Shader shadowShader, int bladeCount)
    : blade(shadowShader, make_vec3(0.0f)),
      bladeCount(bladeCount),
      windTime(0.0f) {
  Shader instancedShader =
      LoadShader("resources/shaders/grass.vs", "resources/shaders/grass.fs");
  instancedShader.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocationAttrib(instancedShader, "instanceTransform");

  Material matInstances = LoadMaterialDefault();
  matInstances.shader = instancedShader;
  planeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
  planeModel.materials[0].shader = shadowShader;
  blade.model.materials[0] = matInstances;

  // Allocate memory for transformations
  transforms = (Matrix*)RL_CALLOC(bladeCount, sizeof(Matrix));

  // Add scale factor for blade size
  float bladeSizeMin = 1.0f;  // Minimum blade size (was implicitly 1.0)
  float bladeSizeMax = 1.3f;  // Maximum blade size
  float bladeVertical = 1.5f;
  int area = 15;

  for (int i = 0; i < bladeCount; i++) {
    Vector3 position = (Vector3){GetRandomFloat(-area, area), 0.0f,
                                 GetRandomFloat(-area, area)};
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);

    // Add random scaling to each blade
    float randomSize = GetRandomFloat(bladeSizeMin, bladeSizeMax);
    Matrix scale =
        MatrixScale(randomSize, randomSize * bladeVertical, randomSize);

    Vector3 axis = Vector3Normalize((Vector3){1.0, 0.0, 0.0});
    float angle = GetRandomFloat(150, 180) * DEG2RAD;
    Matrix rotation = MatrixRotate(axis, angle);

    // Combine all transformations: Scale -> Rotate -> Translate
    Matrix scaleRotate = MatrixMultiply(rotation, scale);
    transforms[i] = MatrixMultiply(scaleRotate, translation);

    blade.model.materials[0].shader.locs[SHADER_LOC_VECTOR_VIEW] =
        GetShaderLocation(blade.model.materials[0].shader, "windParams");
  }
}

void Terrain::update(float dt) {
  windTime += dt;

  // Update wind parameters in shader
  float windStrength = 0.3f;    // Maximum rotation angle in radians
  float windFrequency = 10.0f;  // How many waves across the field
  float windSpeed = 4.5f;       // How fast the wind moves

  Vector4 windParams = {windStrength, windFrequency, windSpeed, windTime};

  SetShaderValue(blade.model.materials[0].shader,
                 blade.model.materials[0].shader.locs[SHADER_LOC_VECTOR_VIEW],
                 &windParams, SHADER_UNIFORM_VEC4);
}

void Terrain::draw(GameState& GameState) {
  std::vector<Matrix> visibleTransforms;
  visibleTransforms.reserve(bladeCount);

  float bladeRadius = 0.5f;  // Approximate radius for each grass blade

  DrawModelEx(planeModel, (Vector3){0.0f, -0.5f, 0.0f}, Vector3Zero(), 0.0f,
              (Vector3){80.0f, 1.0f, 80.0f}, (Color){43, 31, 24, 255});

  for (int i = 0; i < bladeCount; i++) {
    Vector3 bladePos = {transforms[i].m12, transforms[i].m13,
                        transforms[i].m14};

    if (RenderUtils::is_in_camera_view(bladePos, bladeRadius, GameState.camera,
                                       GameState.screenWidth,
                                       GameState.screenHeight)) {
      visibleTransforms.push_back(transforms[i]);
    }
  }

  // Draw only the visible instances
  DrawMeshInstanced(blade.model.meshes[0], blade.model.materials[0],
                    visibleTransforms.data(), visibleTransforms.size());
}
