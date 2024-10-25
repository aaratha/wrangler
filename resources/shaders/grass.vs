#version 330

// Input vertex attributes
in vec3 vertexPosition;      // Vertex position relative to origin
in vec2 vertexTexCoord;      // Texture coordinate of vertex
in mat4 instanceTransform;   // Model transformation matrix for each instance

// Input uniform values
uniform mat4 mvp;            // Model-View-Projection matrix

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec3 fragPosition;

void main()
{
    // Pass texture coordinate to fragment shader
    fragTexCoord = vertexTexCoord;

    // Calculate transformed position in world space
    vec4 worldPosition = instanceTransform * vec4(vertexPosition, 1.0);
    fragPosition = worldPosition.xyz;

    // Calculate final vertex position in screen space
    gl_Position = mvp * worldPosition;
}
