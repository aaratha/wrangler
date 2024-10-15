#include "render_utils.h"

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

void UnloadResources(
    Shader shadowShader,
    Player player,
    std::vector<Animal> animals,
    RenderTexture2D shadowMap,
    Model cube,
    Shader dofShader,
    RenderTexture2D dofTexture
) {
    UnloadShader(shadowShader);
    UnloadModel(player.model);
    UnloadModel(player.tether.model);
    for (auto& animal : animals) {
        UnloadModel(animal.model);
    }
    UnloadShadowmapRenderTexture(shadowMap);
    UnloadModel(cube);
    UnloadShader(dofShader);
    UnloadRenderTexture(dofTexture);
}
