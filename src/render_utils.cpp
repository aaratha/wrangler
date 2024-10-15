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
