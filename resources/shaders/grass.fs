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

    // Normalize Y position to [0,1] range
    float gradientFactor = clamp(fragPosition.y + 0.2, 0.0, 1.0);

    // Define four colors for the gradient
    vec4 colorBottom = vec4(0.1, 0.2, 0.1, 1.0);    // Dark green
    vec4 colorLower = vec4(0.3, 0.6, 0.3, 1.0);     // Medium dark green
    vec4 colorUpper = vec4(0.5, 0.8, 0.5, 1.0);     // Medium light green
    vec4 colorTop = vec4(0.6, 0.9, 0.6, 1.0);       // Light green

    // Create a smooth gradient between all four colors
    vec4 gradientColor;
    if(gradientFactor < 0.33) {
        gradientColor = mix(colorBottom, colorLower, gradientFactor * 3.0);
    }
    else if(gradientFactor < 0.66) {
        gradientColor = mix(colorLower, colorUpper, (gradientFactor - 0.33) * 3.0);
    }
    else {
        gradientColor = mix(colorUpper, colorTop, (gradientFactor - 0.66) * 3.0);
    }

    // Combine diffuse color, texture color, and gradient color
    finalColor = colDiffuse * texelColor * gradientColor;
}
