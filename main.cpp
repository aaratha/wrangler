/*******************************************************************************************
*
*   raylib [shapes] example - collision area
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2019 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib-cpp.hpp"
#include <limits>
#include <vector>

namespace rl = raylib;  // Add this line after includes
using vec3 = rl::Vector3;  // Add this line after namespace alias


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

  Tether() {
    pos = vec3{0.0, 0.0, 10.0};
    targ = vec3{0.0, 0.0, 10.0};
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
            0.0f,  // We're projecting onto XZ plane, so y = 0
            ray.position.z + ray.direction.z * t
        };

        // Lerp to the intersection point
        pos = lerp3D(pos, intersection, 0.6f);
    }

  void draw() {
    DrawCube(pos, 1.0f, 1.0f, 1.0f, BLUE);
  }
};

class Rope {
public:
  vec3 start;
  vec3 end;
  float thickness;
  int num_points;
  std::vector<vec3> points;

  int sides = 10;

    Rope(vec3 playerPos, vec3 tetherPos, float thickness, int num_points)
        : start(playerPos), end(tetherPos), thickness(thickness), num_points(num_points) {
        points = std::vector<vec3>(num_points); // Initialize vector with num_points
        init_points(num_points);
    }

    void init_points(int num_points) {
        for (int i = 0; i < num_points; ++i) {
            float t = static_cast<float>(i) / (num_points - 1); // Normalized factor
            points[i] = Vector3Lerp(start, end, t); // Calculate the position at t
        }
    }

  void update(vec3 playerPos, vec3 tetherPos) {
    start = playerPos;
    end = tetherPos;
    init_points(num_points);
  }

  void draw() {
    for (int i=0; i<num_points -1; i++) {
      DrawCylinderEx(
        points[i],
        points[i+1],
        thickness,
        thickness-0.1,
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
    //Rope rope = Rope(pos, tether);

    // Constructor
    Player(vec3 startPos, float speed)
        : pos(startPos), targ(startPos), movementSpeed(speed),
        tether(), rope(pos, targ, 0.3, 5), com(0.0, 0.0, 5.0) {
        weight = 0.3f;
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

        com = Vector3Add(Vector3Scale(pos, 1.0f - weight), Vector3Scale(tether.pos, weight));

    }

  void draw() {
    DrawCube(pos, 2.0f, 2.0f, 2.0f, RED);
    DrawCubeWires(pos, 2.0f, 2.0f, 2.0f, MAROON);

  }

};


const float speed = 0.2;


void update_camera(Camera3D& camera, Player player) {
    camera.target.x = lerp_to(camera.target.x, player.com.x, 0.2f);
    camera.target.z = lerp_to(camera.target.z, player.com.z, 0.2f);
    camera.target.y = 0.0f;
    //camera.position = lerp3D(camera.position, player.com + vec3{0.0, 15.0, 8.0}, 0.9);
    camera.position = player.pos + vec3{0.0, 15.0, 8.0};
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();

  Player player = Player(
    vec3{0.0,0.0,0.0},
    0.1);

    const float rate = 0.4;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = {0.0, 15.0, 5.0};  // Camera position
    camera.target = vec3{0.0, 10.0, 10.0};      // Camera looking at point
    camera.up = vec3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
    // cameraMode = CAMERA_THIRD_PERSON;


    SetTargetFPS(70);               // Set our game to run at 60 frames-per-second
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
      update_camera(camera, player);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);


                player.draw();
                player.tether.draw();

                // DrawSphere(player.com, 0.3f, BLUE); // com visualizer

                DrawGrid(20, 1.0f);

                player.rope.draw();

            EndMode3D();

            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }


    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
