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

float lerp_to(float position, float target, float rate) {
  return position + (target - position) * rate;
}

raylib::Vector3 lerp3D(raylib::Vector3 position, raylib::Vector3 target, float rate) {
  return position + (target - position) * rate;
}

class Player {
public:
    raylib::Vector3 pos;
    raylib::Vector3 targ;
    float movementSpeed;

    // Constructor
    Player(raylib::Vector3 startPos, float speed)
        : pos(startPos), targ(startPos), movementSpeed(speed) {
    }

    // Method to handle input and move the player
    void Update() {
        if (IsKeyDown(KEY_W)) {
            targ += raylib::Vector3(0.0f, 0.0f, -movementSpeed);  // Move forward
        }
        if (IsKeyDown(KEY_S)) {
            targ += raylib::Vector3(0.0f, 0.0f, movementSpeed);   // Move backward
        }
        if (IsKeyDown(KEY_A)) {
            targ += raylib::Vector3(-movementSpeed, 0.0f, 0.0f);  // Move left
        }
        if (IsKeyDown(KEY_D)) {
            targ += raylib::Vector3(movementSpeed, 0.0f, 0.0f);   // Move right
        }

        // Update the target for camera following
        pos = lerp3D(pos, targ, 0.1);
    }
};


const float speed = 0.2;
raylib::Vector3 Position = raylib::Vector3{0.0, 0.0, 0.0};
raylib::Vector3 Target = Position;


void update_camera(Camera3D& camera) {
    float targetX = camera.target.x + GetMouseDelta().x * 0.5f;
    float targetZ = camera.target.z + GetMouseDelta().y * 0.5f;

    // Now lerp to those target positions
    camera.target.x = lerp_to(camera.target.x, targetX, 0.02f);
    camera.target.z = lerp_to(camera.target.z, targetZ, 0.02f);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();

  Player player = Player(
    raylib::Vector3{0.0,1.0,0.0},
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
    camera.target = raylib::Vector3{0.0, 10.0, 10.0};      // Camera looking at point
    camera.up = raylib::Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
    // cameraMode = CAMERA_THIRD_PERSON;


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    SetExitKey(KEY_NULL);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

      camera.target = lerp3D(camera.target, player.pos, 0.05);
      camera.position = lerp3D(camera.position, player.pos + raylib::Vector3{0.0, 15.0, 8.0}, 0.1);
      player.Update();
      update_camera(camera);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawCube(player.pos, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(player.pos, 2.0f, 2.0f, 2.0f, MAROON);

                DrawGrid(20, 1.0f);

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
