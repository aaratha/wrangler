#version 330

// Input vertex attributes
in vec3 vertexPosition; // vertex position relative to origin
in vec2 vertexTexCoord; // texture coord of vertex
in mat4 instanceTransform; // model transformation matrix

// Input uniform values
uniform mat4 mvp; // model-view-projection

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;

void main()
{
    // Pass texture coord
    fragTexCoord = vertexTexCoord;
    // Compute MVP for current instance
    mat4 mvpi = mvp*instanceTransform;
    // Calculate final vertex position
    gl_Position = mvpi*vec4(vertexPosition, 1.0);
}
