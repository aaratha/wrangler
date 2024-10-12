#include "raylib-cpp.hpp"
#include "raymath.h"
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

namespace rl = raylib;  // Add this line after includes
using vec3 = rl::Vector3;  // Add this line after namespace alias


const float speed = 0.2;
const vec3 CAMERA_OFFSET = {0.0, 15.0, 8.0};

float lerp_to(float position, float target, float rate) {
  return position + (target - position) * rate;
}

vec3 lerp3D(vec3 position, vec3 target, float rate) {
  return position + (target - position) * rate;
}

class Tether {
public:
  vec3 pos;
  vec3 targ;
  Shader shader;
  Model tetherModel;

  Tether(Shader shader) : shader(shader) {
    pos = vec3{0.0, 1.0, 10.0};
    targ = vec3{0.0, 0.0, 10.0};

    tetherModel = LoadModelFromMesh(GenMeshSphere(0.5, 20, 20));
    tetherModel.materials[0].shader = shader;
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
        tetherModel.transform = MatrixTranslate(pos.x, pos.y, pos.z);
    }

  void draw() {
    DrawModel(tetherModel, Vector3Zero(), 1.0f, BLUE);
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

	for (int i=1; i<num_points-1; ++i) {
		vec3 vec2prev = points[i] - points[i - 1];
		vec3 vec2next = points[i + 1] - points[i];
		float dist2prev = Vector3Length(vec2prev);
		float dist2next = Vector3Length(vec2next);
		if (dist2prev > constraint) {
			vec2prev = Vector3Scale(Vector3Normalize(vec2prev), constraint);
		}
		if (dist2next > constraint) {
			vec2next = Vector3Scale(Vector3Normalize(vec2next), constraint);
		}
		points[i] = (points[i-1] + vec2prev + points[i + 1] - vec2next) / 2;
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
    Model playerModel;
    //Rope rope = Rope(pos, tether);

    // Constructor
    Player(vec3 startPos, float speed, Shader shader)
        : pos(startPos), targ(startPos), movementSpeed(speed),
        tether(shader), rope(pos, targ, 0.1, 15, 0.1f), com(0.0, 0.0, 5.0), shader(shader) {

        weight = 0.3f;
        playerModel = LoadModelFromMesh(GenMeshCube(2.0, 2.0, 2.0));
        playerModel.materials[0].shader = shader;
    }

    float weight = 0.3;

    // Method to handle input and move the player
    void update() {
        if (IsKeyDown(KEY_W)) {
            targ += vec3(0.0f, 0.0f, -movementSpeed);  // Move forward
        }
        if (IsKeyDown(KEY_S)) {
            targ += vec3(0.0f, 0.0f, movementSpeed);   // Move backward
        }
        if (IsKeyDown(KEY_A)) {
            targ += vec3(-movementSpeed, 0.0f, 0.0f);  // Move left
        }
        if (IsKeyDown(KEY_D)) {
            targ += vec3(movementSpeed, 0.0f, 0.0f);   // Move right
        }

        pos = lerp3D(pos, targ, 0.4);

        playerModel.transform = MatrixTranslate(pos.x, pos.y, pos.z);

        com = Vector3Add(Vector3Scale(pos, 1.0f - weight), Vector3Scale(tether.pos, weight));

    }

    void draw() {
        // Draw the cube with WHITE as base color (shader will modify it)
        DrawModel(playerModel, Vector3Zero(), 1.0f, RED);
    }

};


class Animal {
public:
    vec3 pos;
    vec3 targ;
    float speed;
    Shader shader;
    Model model;

    Animal(vec3 pos, float speed, Shader shader) :
        pos(pos), speed(speed), shader(shader)
    {
        targ = pos;
        model = LoadModelFromMesh(GenMeshSphere(0.5, 20, 20));
        model.materials[0].shader = shader;
    }

    void setNewRandomTarget() {
        // Define the range for random movement (e.g., [-1.0, 1.0])
        float rangep = 0.2f;

        // Generate a random value within the range for both x and z coordinates
        float rangeX = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * rangep;
        float rangeZ = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * rangep;

        // Update the target position with the new random values
        targ.x = targ.x + rangeX;
        targ.z = targ.z + rangeZ;
    }

    void update() {
        setNewRandomTarget();
        pos = lerp3D(pos, targ, 0.3);

        model.transform = MatrixTranslate(pos.x, pos.y, pos.z);
    }

    void draw() {
        DrawModel(model, Vector3Zero(), 1.0f, YELLOW);
    }
};


void update_camera(Camera3D& camera, Player player) {
    camera.target.x = lerp_to(camera.target.x, player.com.x, 0.2f);
    camera.target.z = lerp_to(camera.target.z, player.com.z, 0.2f);
    camera.target.y = 0.0f;
    //camera.position = lerp3D(camera.position, player.com + vec3{0.0, 15.0, 8.0}, 0.9);
    camera.position = player.pos + CAMERA_OFFSET;

    camera.fovy += GetMouseWheelMove();
    camera.fovy = Clamp(camera.fovy, 20.0f, 100.0f);
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

    rl::Shader shader (TextFormat("resources/shaders/lighting.vs", GLSL_VERSION),
                       TextFormat("resources/shaders/lighting.fs", GLSL_VERSION));
    // Get some required shader locations
    shader.locs[SHADER_LOC_VECTOR_VIEW] = shader.GetLocation("viewPos");
    // NOTE: "matModel" location name is automatically assigned on shader loading,
    // no need to get the location again if using that uniform name
    //shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

    // Ambient light level (some basic lighting)
    int ambientLoc = shader.GetLocation("ambient");
    std::array<float, 4> ambientValues = {0.9f, 0.9f, 0.9f, 1.0f};
    shader.SetValue(ambientLoc, ambientValues.data(), SHADER_UNIFORM_VEC4);

    // Create lights
    std::array<Light, MAX_LIGHTS> lights = {
        CreateLight(LIGHT_POINT, (Vector3) {
            -10, 40, 10},
            Vector3Zero(),
            (Color){255, 250, 189, 255},
            shader
        ),
    };


    Player player = Player(
        vec3{0.0,1.0,0.0},
        0.1,
        shader);

    Animal animal = Animal({10.0, 0.5, 10.0}, 5.0, shader);


    Model planeModel = LoadModelFromMesh(GenMeshPlane(50.0, 50.0, 2, 2));
    planeModel.materials[0].shader = shader;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    SetExitKey(KEY_NULL);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

      player.tether.update(camera);
      player.update();
      player.rope.update(player.pos, player.tether.pos);
      animal.update();
      update_camera(camera, player);

      std::array<float, 3> cameraPos = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos.data(), SHADER_UNIFORM_VEC3);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            BeginShaderMode(shader);

            // Update light values (ensure this is called in the main game loop)
                player.draw();
                player.tether.draw();
                animal.draw();

                DrawModel(planeModel, Vector3Zero(), 1.0f, (Color){56, 186, 95, 255});
                // DrawSphere(player.com, 0.3f, BLUE); // com visualizer

                // DrawGrid(20, 1.0f);
                //DrawModel(floorModel, (Vector3){ 0.0f, -1.0f, 0.0f }, 1.0f, WHITE);

                player.rope.draw();

            EndShaderMode();
            EndMode3D();

            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }


    // De-Initialization
    UnloadShader(shader);   // Unload shader
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
