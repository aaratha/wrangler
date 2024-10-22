#include "render_utils.hpp"

#include <string>

#include "raygui.h"

namespace RenderUtils {

void InitializeWindow(int &screenWidth, int &screenHeight) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(screenWidth, screenHeight, "Wrangler");
}

Camera3D SetupCamera() {
  Camera3D camera = {0};
  camera.position = CAMERA_OFFSET;
  camera.target = vec3{0.0, 10.0, 10.0};
  camera.up = vec3{0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
  return camera;
}

Camera3D SetupLightCamera() {
  Camera3D lightCam = (Camera3D){0};
  lightCam.position = Vector3Normalize(
      vec3{-10.0, 70.0, 10.0});  // Vector3Scale(lightDir, -15.0f);
  lightCam.target = Vector3Zero();
  // Use an orthographic projection for directional lights
  lightCam.projection = CAMERA_ORTHOGRAPHIC;
  lightCam.up = (Vector3){0.0f, 1.0f, 0.0f};
  lightCam.fovy = 80.0f;
  return lightCam;
}

RenderTexture2D LoadShadowmapRenderTexture(int width, int height) {
  RenderTexture2D target = {0};

  target.id = rlLoadFramebuffer(0, 0);  // Load an empty framebuffer
  target.texture.width = width;
  target.texture.height = height;

  if (target.id > 0) {
    rlEnableFramebuffer(target.id);

    // Create depth texture
    // We don't need a color texture for the shadowmap
    target.depth.id = rlLoadTextureDepth(width, height, false);
    target.depth.width = width;
    target.depth.height = height;
    target.depth.format = 19;  // DEPTH_COMPONENT_24BIT?
    target.depth.mipmaps = 1;

    // Attach depth texture to FBO
    rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH,
                        RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if fbo is complete with attachments (valid)
    if (rlFramebufferComplete(target.id))
      TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully",
               target.id);

    rlDisableFramebuffer();
  } else
    TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

  return target;
}

// Unload shadowmap render texture from GPU memory (VRAM)
void UnloadShadowmapRenderTexture(RenderTexture2D target) {
  if (target.id > 0) {
    // NOTE: Depth texture/renderbuffer is automatically
    // queried and deleted before deleting framebuffer
    rlUnloadFramebuffer(target.id);
  }
}

void update_camera(GameState &GameState) {
  float fov_ext = 70.0;
  float fov_rest = 60.0;
  GameState.camera.target.x =
      lerp_to(GameState.camera.target.x, GameState.player->com.x, 0.2f);
  GameState.camera.target.z =
      lerp_to(GameState.camera.target.z, GameState.player->com.z, 0.2f);
  GameState.camera.target.y = 0.0f;
  // GameState.camera.position = lerp3D(GameState.camera.position,
  // GameState.player.com + vec3{0.0, 15.0, 8.0}, 0.9);
  GameState.camera.position = GameState.player->pos + CAMERA_OFFSET;
  GameState.camera.position.x = GameState.player->com.x + CAMERA_OFFSET.x;
  if (GameState.itemActive == 0) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        GameState.camera.fovy < fov_ext) {
      GameState.camera.fovy = lerp_to(GameState.camera.fovy, fov_ext, 0.1f);
    } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
               GameState.camera.fovy > fov_rest) {
      GameState.camera.fovy = lerp_to(GameState.camera.fovy, fov_rest, 0.1f);
    }
  }

#if defined(_WIN32) || defined(_WIN64)
  GameState.camera.fovy -= 3 * GetMouseWheelMove();
#else
  GameState.camera.fovy -= GetMouseWheelMove();
#endif
  GameState.camera.fovy = Clamp(GameState.camera.fovy, 20.0f, 100.0f);
  GameState.camera.fovy = Clamp(GameState.camera.fovy, 20.0f, 100.0f);
}

bool is_in_camera_view(const Vector3 &position, float radius,
                       const Camera &camera, int screenWidth,
                       int screenHeight) {
  // Convert world position to screen space
  Vector2 screenPos = GetWorldToScreen(position, camera);

  // Check if the screen position is within the screen bounds, accounting for
  // the object's radius
  bool is_visible =
      (screenPos.x + radius > 0) && (screenPos.x - radius < screenWidth) &&
      (screenPos.y + radius > 0) && (screenPos.y - radius < screenHeight);

  return is_visible;
}

void draw_scene(GameState &GameState) {
  GameState.terrain->draw();
  GameState.player->draw();
  GameState.player->tether.draw();
  GameState.player->rope.draw();

  for (auto &animal : GameState.animals) {
    if (is_in_camera_view(animal->pos, animal->species.radius, GameState.camera,
                          GameState.screenWidth, GameState.screenHeight))
      animal->draw();
  }
  GameState.fence->draw(GameState);
  for (const auto &pen : GameState.pens) {
    if (pen) {               // Check if the unique_ptr is not null
      pen->draw(GameState);  // Call the draw method of Pen
    }
  }
  // GameState.pens.draw();
}

void UnloadResources(Shader shadowShader, RenderTexture2D shadowMap,
                     GameState &GameState, Shader dofShader,
                     RenderTexture2D dofTexture) {
  UnloadShader(shadowShader);
  UnloadModel(GameState.player->model);
  UnloadModel(GameState.player->tether.model);
  for (auto &animal : GameState.animals) {
    UnloadModel(animal->model);
  }
  UnloadShadowmapRenderTexture(shadowMap);
  UnloadModel(GameState.terrain->model);
  UnloadShader(dofShader);
  UnloadRenderTexture(dofTexture);
}

RenderTexture2D SetupDofTexture(int screenWidth, int screenHeight) {
  RenderTexture2D dofTexture = LoadRenderTexture(screenWidth, screenHeight);
  if (dofTexture.id == 0) {
    throw std::runtime_error("Failed to create dofTexture");
  }
  return dofTexture;
}

rl::Shader SetupDofShader(int screenWidth, int screenHeight) {
  rl::Shader dofShader(0, TextFormat("resources/shaders/dof.fs", GLSL_VERSION));
  if (dofShader.id == 0) {
    throw std::runtime_error("Failed to compile DoF shader");
  }

  float blurRadius = 3.0f;
  float resolution[2] = {(float)screenWidth, (float)screenHeight};
  SetShaderValue(dofShader, GetShaderLocation(dofShader, "resolution"),
                 resolution, SHADER_UNIFORM_VEC2);
  SetShaderValue(dofShader, GetShaderLocation(dofShader, "radius"), &blurRadius,
                 SHADER_UNIFORM_FLOAT);

  return dofShader;
}

rl::Shader SetupShadowShader(vec3 &lightDir) {
  rl::Shader shadowShader(
      TextFormat("resources/shaders/lighting.vs", GLSL_VERSION),
      TextFormat("resources/shaders/lighting.fs", GLSL_VERSION));
  shadowShader.locs[SHADER_LOC_VECTOR_VIEW] =
      GetShaderLocation(shadowShader, "viewPos");
  Color lightColor = WHITE;
  Vector4 lightColorNormalized = ColorNormalize(lightColor);
  int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
  int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
  SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
  SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized,
                 SHADER_UNIFORM_VEC4);
  int ambientLoc = GetShaderLocation(shadowShader, "ambient");
  float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
  SetShaderValue(shadowShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);
  int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
  int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
  int shadowMapResolution = SHADOWMAP_RESOLUTION;
  SetShaderValue(shadowShader,
                 GetShaderLocation(shadowShader, "shadowMapResolution"),
                 &shadowMapResolution, SHADER_UNIFORM_INT);

  return shadowShader;
}

void RenderShadowMap(Shader shadowShader, RenderTexture2D &shadowMap,
                     Camera3D &lightCam, GameState &GameState) {
  BeginTextureMode(shadowMap);
  ClearBackground(WHITE);
  BeginMode3D(lightCam);
  Matrix lightViewProj =
      MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
  SetShaderValueMatrix(shadowShader, GetShaderLocation(shadowShader, "lightVP"),
                       lightViewProj);
  RenderUtils::draw_scene(GameState);
  EndMode3D();
  EndTextureMode();
}

void RenderSceneToTexture(RenderTexture2D &dofTexture, Camera3D &camera,
                          rl::Shader &shadowShader, RenderTexture2D &shadowMap,
                          GameState &GameState) {
  BeginTextureMode(dofTexture);
  ClearBackground(RAYWHITE);

  rlEnableShader(shadowShader.id);
  int slot = 10;
  rlActiveTextureSlot(10);
  rlEnableTexture(shadowMap.depth.id);
  rlSetUniform(GetShaderLocation(shadowShader, "shadowMap"), &slot,
               SHADER_UNIFORM_INT, 1);

  rlDisableShader();
  BeginMode3D(camera);
  RenderUtils::draw_scene(GameState);
  EndMode3D();

  EndTextureMode();
}

void HandleWindowResize(GameState &GameState, int &screenWidth,
                        int &screenHeight, RenderTexture2D &dofTexture,
                        Shader &dofShader) {
  if (IsWindowResized()) {
    screenWidth = GetScreenWidth();
    GameState.screenWidth = screenWidth;
    screenHeight = GetScreenHeight();
    GameState.screenHeight = screenHeight;

    // Re-create the DOF texture with the new screen size
    UnloadRenderTexture(dofTexture);
    dofTexture = RenderUtils::SetupDofTexture(screenWidth, screenHeight);

    // Re-apply the resolution to the shader
    float resolution[2] = {(float)screenWidth, (float)screenHeight};
    SetShaderValue(dofShader, GetShaderLocation(dofShader, "resolution"),
                   resolution, SHADER_UNIFORM_VEC2);

    // Update the viewport to match the new window size
    rlViewport(0, 0, screenWidth, screenHeight);
  }
}

void DrawGUI(GameState &GameState, int &screenWidth, int &screenHeight) {
  float width = 40.0;
  float height = 40.0;
  float margin = 20.0;
  float textWidth = 200.0;
  float textHeight = 100.0;
  std::string labelText = "Coins: " + std::to_string(GameState.coins);
  GuiToggleGroup(
      (Rectangle){static_cast<float>(screenWidth - width - margin),
                  static_cast<float>(screenHeight - (4.05 * height) - margin),
                  width, height},
      "#1#\n#3#\n#8#\n#23#", &GameState.itemActive);
  GuiLabel((Rectangle){static_cast<float>(width / 2 + margin),
                       static_cast<float>(margin), textWidth, textHeight},
           labelText.c_str());
}
}  // namespace RenderUtils
