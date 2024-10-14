#include "raylib-cpp.hpp"
#include "raymath.h"
#include "rlgl.h"
#include <random>
#include <limits>
#include <vector>
#include <array>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define SHADOWMAP_RESOLUTION 2048


namespace rl = raylib;  // Add this line after includes
using vec3 = rl::Vector3;  // Add this line after namespace alias


RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);
void DrawScene(Model cube, Model robot);


const float speed = 0.2;
const vec3 CAMERA_OFFSET = {0.0, 15.0, 8.0};

float lerp_to(float position, float target, float rate) {
  return position + (target - position) * rate;
}

vec3 lerp3D(vec3 position, vec3 target, float rate) {
  return position + (target - position) * rate;
}

float GetRandomFloat(float min, float max) {
    return min + (max - min) * ((float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX);
}

class Tether {
public:
  vec3 pos;
  vec3 targ;
  Shader shader;
  Model model;

  Tether(Shader shader) : shader(shader) {
    pos = vec3{0.0, 1.0, 10.0};
    targ = vec3{0.0, 0.0, 10.0};

    model = LoadModelFromMesh(GenMeshSphere(0.5, 20, 20));
    model.materials[0].shader = shader;
  }

    void update(const Camera3D& camera) {
        // Get mouse position
        Vector2 mousePos = GetMousePosition();

        // Get the ray from the mouse position
        Ray ray = GetMouseRay(mousePos, camera);

        // Calculate intersection with XZ plane (Y = 0)
        // Using the formula: t = -plane.y / ray.direction.y
        float t = -ray.position.y / ray.direction.y;

        // Get the intersection point
        vec3 intersection = {
            ray.position.x + ray.direction.x * t,
            1.0f,  // We're projecting onto XZ plane, so y = 0
            ray.position.z + ray.direction.z * t
        };

        // Lerp to the intersection point
        pos = lerp3D(pos, intersection, 0.3f);
        model.transform = MatrixTranslate(pos.x, pos.y, pos.z);
    }

  void draw() {
    DrawModel(model, Vector3Zero(), 1.0f, BLUE);
  }
};

class Rope {
public:
  vec3 start;
  vec3 end;
  float thickness;
  int num_points;
  float constraint;
  std::vector<vec3> points;
  std::vector<vec3> velocities;  // MAX_POINTS should be the number of points in the rope
  float friction = 0.98f;  // Friction factor (close to 1.0 means low friction, close to 0 means high friction)


  int sides = 10;

    Rope(
      vec3 playerPos,
      vec3 tetherPos,
      float thickness,
      int num_points,
      float constraint
    ) : start(playerPos),
        end(tetherPos),
        thickness(thickness),
        num_points(num_points),
        constraint(constraint) {

      points = std::vector<vec3>(num_points); // Initialize vector with num_points
      velocities = std::vector<vec3>(num_points);
      init_points();
    }

    void init_points() {
        for (int i = 0; i < num_points; ++i) {
            float t = static_cast<float>(i) / (num_points - 1); // Normalized factor
            points[i] = Vector3Lerp(start, end, t); // Calculate the position at t
        }
    }


    void update(vec3 playerPos, vec3 tetherPos) {
        start = playerPos;
        end = tetherPos;
        points[0] = start;
        points[num_points - 1] = end;

        for (int i = 1; i < num_points - 1; ++i) {
            // Calculate the position difference from neighboring points
            vec3 vec2prev = points[i] - points[i - 1];
            vec3 vec2next = points[i + 1] - points[i];
            float dist2prev = Vector3Length(vec2prev);
            float dist2next = Vector3Length(vec2next);

            // Constrain distances
            if (dist2prev > constraint) {
                vec2prev = Vector3Scale(Vector3Normalize(vec2prev), constraint);
            }
            if (dist2next > constraint) {
                vec2next = Vector3Scale(Vector3Normalize(vec2next), constraint);
            }

            // Calculate the velocity based on the current and next position
            vec3 newPos = (points[i - 1] + vec2prev + points[i + 1] - vec2next) / 2;
            vec3 velocity = newPos - points[i];

            // Apply friction to the velocity
            velocity = Vector3Scale(velocity, friction);

            // Update the point's position using the velocity
            points[i] = points[i] + velocity;

            // Optionally, store this velocity for the next update if needed
            velocities[i] = velocity;
        }
    }


  void draw() {
    for (int i=0; i<num_points -1; i++) {
      DrawCylinderEx(
        points[i],
        points[i+1],
        thickness,
        thickness,
        sides,
        RED
      ); // Draw a cylinder with base at startPos and top at endPos
    }
  }

};

class Player {
public:
    vec3 pos;
    vec3 targ;
    float movementSpeed;
    Tether tether;
    Rope rope;
    vec3 com;
    Shader shader;
    Model model;
    //Rope rope = Rope(pos, tether);

    // Constructor
    Player(vec3 startPos, float speed, Shader shader)
        : pos(startPos), targ(startPos), movementSpeed(speed),
        tether(shader), rope(pos, targ, 0.1, 15, 0.05f), com(0.0, 0.0, 5.0), shader(shader) {

        weight = 0.3f;
        model = LoadModelFromMesh(GenMeshCube(2.0, 2.0, 2.0));
        model.materials[0].shader = shader;
    }

    float weight = 0.1;

    // Method to handle input and move the player
    void update() {
        vec3 direction = vec3(0.0f, 0.0f, 0.0f);  // Movement direction

        if (IsKeyDown(KEY_W)) {
            direction += vec3(0.0f, 0.0f, -1.0f);  // Move forward
        }
        if (IsKeyDown(KEY_S)) {
            direction += vec3(0.0f, 0.0f, 1.0f);   // Move backward
        }
        if (IsKeyDown(KEY_A)) {
            direction += vec3(-1.0f, 0.0f, 0.0f);  // Move left
        }
        if (IsKeyDown(KEY_D)) {
            direction += vec3(1.0f, 0.0f, 0.0f);   // Move right
        }

        // Normalize direction if it's non-zero to prevent diagonal speed boost
        if (Vector3Length(direction) > 0.0f) {
            direction = Vector3Normalize(direction);
        }

        // Apply movement speed
        targ += direction * movementSpeed;

        // Interpolate position smoothly
        pos = lerp3D(pos, targ, 0.4);

        // Update player model transformation
        model.transform = MatrixTranslate(pos.x, pos.y, pos.z);

        // Update center of mass (com)
        com = Vector3Add(Vector3Scale(pos, 1.0f - weight), Vector3Scale(tether.pos, weight));

    }


    void draw() {
        // Draw the cube with WHITE as base color (shader will modify it)
        DrawModel(model, Vector3Zero(), 1.0f, RED);
    }

};


enum class SpeciesType {
    WOLF,
    SHEEP,
    COW
};

class Species {
public:
    SpeciesType type;
    Color color;
    std::string name;

    Species(SpeciesType type) : type(type) {
        switch (type) {
            case SpeciesType::WOLF:
                color = GRAY;
                name = "Wolf";
                break;
            case SpeciesType::SHEEP:
                color = WHITE;
                name = "Sheep";
                break;
            case SpeciesType::COW:
                color = BROWN;
                name = "Cow";
                break;
        }
    }
};

SpeciesType getRandomSpecies() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 2);

    int randomNumber = dis(gen);
    switch (randomNumber) {
        case 0: return SpeciesType::WOLF;
        case 1: return SpeciesType::SHEEP;
        case 2: return SpeciesType::COW;
        default: return SpeciesType::SHEEP; // Fallback, should never happen
    }
}



class Animal {
public:
    vec3 pos;
    vec3 targ;
    float speed;
    Shader shader;
    Model model;
    double lastUpdateTime;
    Species species;

    Animal(vec3 pos, float speed, Shader shader) :
        pos(pos), speed(speed), shader(shader), lastUpdateTime(0), species(getRandomSpecies())
    {
        targ = pos;
        model = LoadModelFromMesh(GenMeshSphere(0.5, 20, 20));
        model.materials[0].shader = shader;
    }

    void setNewRandomTarget() {
        // Define the range for random movement (e.g., [-1.0, 1.0])
        float rangep = 1.0f;

        // Generate a random value within the range for both x and z coordinates
        float rangeX = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * rangep;
        float rangeZ = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * rangep;

        // Update the target position with the new random values
        targ.x = targ.x + rangeX;
        targ.z = targ.z + rangeZ;
    }

    void update() {
        double currentTime = GetTime();
        if (currentTime - lastUpdateTime >= 1.0) {
            setNewRandomTarget();
            lastUpdateTime = currentTime;
        }

        pos = lerp3D(pos, targ, 0.03);
        model.transform = MatrixTranslate(pos.x, pos.y, pos.z);
    }

    void draw() {
        DrawModel(model, Vector3Zero(), 1.0f, species.color);
    }
};


void update_camera(Camera3D& camera, Player player) {
    camera.target.x = lerp_to(camera.target.x, player.com.x, 0.2f);
    camera.target.z = lerp_to(camera.target.z, player.com.z, 0.2f);
    camera.target.y = 0.0f;
    //camera.position = lerp3D(camera.position, player.com + vec3{0.0, 15.0, 8.0}, 0.9);
    camera.position = player.pos + CAMERA_OFFSET;
    camera.position.x = player.com.x + CAMERA_OFFSET.x;

    #if defined(_WIN32) || defined(_WIN64)
        camera.fovy -= 3*GetMouseWheelMove();
    #else
        camera.fovy -= GetMouseWheelMove();
    #endif
    camera.fovy = Clamp(camera.fovy, 20.0f, 100.0f);
    camera.fovy = Clamp(camera.fovy, 20.0f, 100.0f);
}

// Helper function to get the closest point on a line segment to a point
vec3 GetClosestPointOnLineFromPoint(vec3 point, vec3 lineStart, vec3 lineEnd) {
    vec3 line = Vector3Subtract(lineEnd, lineStart);
    float lineLength = Vector3Length(line);
    vec3 lineNormalized = Vector3Scale(line, 1.0f / lineLength);

    float t = Vector3DotProduct(Vector3Subtract(point, lineStart), lineNormalized);
    t = Clamp(t, 0, lineLength);

    return Vector3Add(lineStart, Vector3Scale(lineNormalized, t));
}

// Helper function to check collision between a point and a line segment
bool CheckCollisionPointLine(vec3 point, vec3 lineStart, vec3 lineEnd, float threshold) {
    vec3 closest = GetClosestPointOnLineFromPoint(point, lineStart, lineEnd);
    return Vector3Distance(point, closest) <= threshold;
}

void handle_collisions(Player& player, std::vector<Animal>& animals) {
    const float playerRadius = 1.5f; // Assuming the player cube has a side length of 2.0
    const float animalRadius = 0.5f; // From the Animal constructor
    const float tetherRadius = 0.5f; // From the Tether constructor
    const float ropeSegmentRadius = 0.5f; // From the Rope constructor

    // Player cube vs Animals
    for (auto& animal : animals) {
        if (CheckCollisionSpheres(player.pos, playerRadius, animal.pos, animalRadius)) {
            // Handle player-animal collision
            vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, player.pos));
            float overlap = playerRadius + animalRadius - Vector3Distance(player.pos, animal.pos);
            player.pos = Vector3Subtract(player.pos, Vector3Scale(collisionNormal, overlap * 0.5f));
            animal.pos = Vector3Add(animal.pos, Vector3Scale(collisionNormal, overlap * 0.5f));
        }
    }

    // Player rope vs Animals
    for (auto& animal : animals) {
        for (int i = 0; i < player.rope.num_points - 1; i++) {
            if (CheckCollisionPointLine(animal.pos, player.rope.points[i], player.rope.points[i+1], ropeSegmentRadius)) {
                // Handle rope-animal collision
                vec3 closestPoint = GetClosestPointOnLineFromPoint(animal.pos, player.rope.points[i], player.rope.points[i+1]);
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, closestPoint));
                float overlap = ropeSegmentRadius + animalRadius - Vector3Distance(closestPoint, animal.pos);
                animal.targ = Vector3Add(animal.targ, Vector3Scale(collisionNormal, overlap*0.8));
            }
        }
    }


    // Player tether vs Animals
    for (auto& animal : animals) {
        if (CheckCollisionSpheres(player.tether.pos, tetherRadius, animal.pos, animalRadius)) {
            // Handle tether-animal collision
            vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, player.tether.pos));
            float overlap = tetherRadius + animalRadius - Vector3Distance(player.tether.pos, animal.pos);
            animal.pos = Vector3Add(animal.pos, Vector3Scale(collisionNormal, overlap));
        }
    }

    // Animal vs Animal
    for (size_t i = 0; i < animals.size(); i++) {
        for (size_t j = i + 1; j < animals.size(); j++) {
            if (CheckCollisionSpheres(animals[i].pos, animalRadius, animals[j].pos, animalRadius)) {
                // Handle animal-animal collision
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animals[j].pos, animals[i].pos));
                float overlap = 2 * animalRadius - Vector3Distance(animals[i].pos, animals[j].pos);
                animals[i].pos = Vector3Subtract(animals[i].pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                animals[j].pos = Vector3Add(animals[j].pos, Vector3Scale(collisionNormal, overlap * 0.5f));
            }
        }
    }
}


RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(0,0); // Load an empty framebuffer
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create depth texture
        // We don't need a color texture for the shadowmap
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach depth texture to FBO
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

// Unload shadowmap render texture from GPU memory (VRAM)
void UnloadShadowmapRenderTexture(RenderTexture2D target)
{
    if (target.id > 0)
    {
        // NOTE: Depth texture/renderbuffer is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}

void draw_scene(Model cube, Player player, std::vector<Animal> animals) {
    DrawModelEx(cube,
        Vector3Zero(),
        (Vector3) { 0.0f, -1.0f, 0.0f },
        0.0f,
        (Vector3) { 40.0f, 1.0f, 40.0f },
        (Color) {50, 168, 82, 255}
    );
    player.draw();
    player.tether.draw();
    player.rope.draw();

    for (auto& animal : animals) {
        animal.draw();
    }
}
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();


    const float rate = 0.4;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = CAMERA_OFFSET;  // Camera position
    camera.target = vec3{0.0, 10.0, 10.0};      // Camera looking at point
    camera.up = vec3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
    // cameraMode = CAMERA_THIRD_PERSON;

// Load textures
    // After initializing the camera

    rl::Shader shadowShader (TextFormat("resources/shaders/lighting.vs", GLSL_VERSION),
                       TextFormat("resources/shaders/lighting.fs", GLSL_VERSION));
    // Get some required shader locations
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    Vector3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    Color lightColor = WHITE;
    Vector4 lightColorNormalized = ColorNormalize(lightColor);
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    int ambientLoc = GetShaderLocation(shadowShader, "ambient");
    float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    SetShaderValue(shadowShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);
    int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    int shadowMapResolution = SHADOWMAP_RESOLUTION;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);


    Player player = Player(
        vec3{0.0,1.0,0.0},
        0.2,
        shadowShader);

    std::vector<Animal> animals;
    for (int i = 0; i < 10; i++) {  // Create 10 animals, for example
        animals.push_back(Animal(vec3{GetRandomFloat(-25, 25), 1.0f, GetRandomFloat(-25, 25)}, 5.0f, shadowShader));
    }

    Model cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    cube.materials[0].shader = shadowShader;

    RenderTexture2D shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    // For the shadowmapping algorithm, we will be rendering everything from the light's point of view
    Camera3D lightCam = (Camera3D){ 0 };
    lightCam.position = Vector3Normalize(vec3{-10.0, 70.0, 10.0});//Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();
    // Use an orthographic projection for directional lights
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 50.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
        int fc = 0;

    SetExitKey(KEY_NULL);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

         float dt = GetFrameTime();

      handle_collisions(player, animals);
      player.tether.update(camera);
      player.update();
      player.rope.update(player.pos, player.tether.pos);
        for (auto& animal : animals) {
            animal.update();
        }
      update_camera(camera, player);

        Vector3 cameraPos = camera.position;
        SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);

                const float cameraSpeed = 0.05f;
        if (IsKeyDown(KEY_LEFT))
        {
            if (lightDir.x < 0.6f)
                lightDir.x += cameraSpeed * 60.0f * dt;
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            if (lightDir.x > -0.6f)
                lightDir.x -= cameraSpeed * 60.0f * dt;
        }
        if (IsKeyDown(KEY_UP))
        {
            if (lightDir.z < 0.6f)
                lightDir.z += cameraSpeed * 60.0f * dt;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            if (lightDir.z > -0.6f)
                lightDir.z -= cameraSpeed * 60.0f * dt;
        }
        lightDir = Vector3Normalize(lightDir);
        lightCam.position = Vector3Scale(lightDir, -15.0f);
        SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);


        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            Matrix lightView;
            Matrix lightProj;
            BeginTextureMode(shadowMap);
            ClearBackground(WHITE);
            BeginMode3D(lightCam);
                lightView = rlGetMatrixModelview();
                lightProj = rlGetMatrixProjection();
                draw_scene(cube, player, animals);
            EndMode3D();
            EndTextureMode();
            Matrix lightViewProj = MatrixMultiply(lightView, lightProj);


            ClearBackground(RAYWHITE);

            SetShaderValueMatrix(shadowShader, lightVPLoc, lightViewProj);

            rlEnableShader(shadowShader.id);
            int slot = 10; // Can be anything 0 to 15, but 0 will probably be taken up
            rlActiveTextureSlot(10);
            rlEnableTexture(shadowMap.depth.id);
            rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

            BeginMode3D(camera);

            // Update light values (ensure this is called in the main game loop)
                draw_scene(cube, player, animals);

            EndMode3D();

            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }


    // De-Initialization
    UnloadShader(shadowShader);
    UnloadModel(player.model);
    UnloadModel(player.tether.model);
    for (auto& animal : animals) {
        UnloadModel(animal.model);
    }
    UnloadShadowmapRenderTexture(shadowMap);
    //UnloadModel(cube);
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
