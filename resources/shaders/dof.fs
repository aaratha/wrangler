#version 330

uniform sampler2D texture0;  // The render texture
uniform vec2 resolution;     // Screen resolution
uniform float radius;     // Maximum blur radius at top/bottom

in vec2 fragTexCoord;
out vec4 finalColor;

void main() {
    vec2 texelSize = 1.0 / resolution;  // Size of a single texel
    vec3 color = vec3(0.0);

    // Gaussian kernel weights for a 9x9 kernel (reusing for both axes)
    float kernel[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    // Correct Y-coordinate (OpenGL texture coordinates start at bottom-left)
    vec2 correctedCoord = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);

    // Calculate the vertical distance from the center of the screen (normalized)
    float verticalDistance = abs((fragTexCoord.y - 0.5) * 2.0);

    // Interpolate the blur radius: 0 at the center, maxRadius at the top/bottom
    float dynamicRadius = mix(0.0, radius, verticalDistance);

    // Apply blur by sampling around the pixel using the dynamically changing radius
    for (int i = -4; i <= 4; i++) {
        for (int j = -4; j <= 4; j++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize * dynamicRadius;
            vec3 sampleColor = texture(texture0, correctedCoord + offset).rgb;

            // Weight is determined by both the x and y directions
            float weight = kernel[abs(i)] * kernel[abs(j)];
            color += sampleColor * weight;
        }
    }

    finalColor = vec4(color, 1.0);
}
