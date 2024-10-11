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

raylib::Vector3 lerp_to(raylib::Vector3 position, raylib::Vector3 target, float rate) {
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
        pos = lerp_to(pos, targ, 0.1);
    }
};


const float speed = 0.2;
raylib::Vector3 Position = raylib::Vector3{0.0, 0.0, 0.0};
raylib::Vector3 Target = Position;



//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {

  Player player = Player(
    raylib::Vector3{0.0,0.0,0.0},
    0.1);

  const float rate = 0.4;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = {0.0, 10.0, 10.0};  // Camera position
    camera.target = raylib::Vector3{0.0, 10.0, 10.0};      // Camera looking at point
    camera.up = raylib::Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

      camera.target = lerp_to(camera.target, player.pos, 0.1);
      camera.position = lerp_to(camera.position, player.pos + raylib::Vector3{0.0, 10.0, 10.0}, 0.1);
      player.Update();
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawCube(player.pos, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(player.pos, 2.0f, 2.0f, 2.0f, MAROON);

                DrawGrid(10, 1.0f);

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
