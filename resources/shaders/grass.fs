#version 330

// Input from vertex shader
in vec3 fragPosition;
in vec2 fragTexCoord;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Fetch texel color from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Compute gradient factor using the Y position of fragPosition


        float gradientFactor = clamp(fragPosition.y + 0.2, 0.0, 1.0);


    // Use gradientFactor to blend from dark green to light green
    vec4 gradientColor = mix(vec4(0.2, 0.4, 0.2, 1.0), vec4(0.5, 1.0, 0.5, 1.0), gradientFactor);


    // Combine diffuse color, texture color, and gradient color
    finalColor = colDiffuse * texelColor * gradientColor;
}
