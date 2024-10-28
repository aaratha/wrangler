#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in mat4 instanceTransform;

// Uniforms for camera, transformation, and wind animation
uniform mat4 mvp;
uniform vec4 windParams;  // x: strength, y: frequency, z: speed, w: time
uniform vec3 cameraPos;   // Camera position
uniform vec3 cameraDir;   // Normalized camera direction
uniform float fovy;       // Field of view (vertical, in radians)
uniform float screenWidth;
uniform float screenHeight;

// Output to the fragment shader
out vec2 fragTexCoord;
out vec3 fragPosition;

void main() {
  fragTexCoord = vertexTexCoord;
  vec3 position = vertexPosition;

  // Transform the position to clip space to check visibility
  vec4 worldPosition = instanceTransform * vec4(position, 1.0);
  vec4 clipPos = mvp * worldPosition;

  // Convert to normalized device coordinates (NDC)
  vec3 ndc = clipPos.xyz / clipPos.w;

  // Check if the point is within the view frustum (-1 to 1 in NDC)
  if (abs(ndc.x) <= 1.0 && abs(ndc.y) <= 1.0 && ndc.z <= 1.0) {
    // Point is visible, apply wind effect
    vec3 basePos = vec3(instanceTransform[3][0], instanceTransform[3][1],
                        instanceTransform[3][2]);
    if (abs(ndc.x) <= 1.0 && abs(ndc.y) <= 0.8 && ndc.z <= 1.0) {
      float windStrength = windParams.x;
      float windFrequency = windParams.y;
      float windSpeed = windParams.z;
      float time = windParams.w;

      float oscillation =
          sin(windFrequency * basePos.x + windSpeed * time) *
          cos(windFrequency * basePos.z + windSpeed * time * 0.7);
      float windDisplacement = oscillation * windStrength * position.y;

      // Apply wind displacement
      position.x += windDisplacement;
      position.z += windDisplacement * 0.5;
    }

    // Calculate final world and clip space positions
    vec4 finalWorldPosition = instanceTransform * vec4(position, 1.0);
    fragPosition = finalWorldPosition.xyz;
    gl_Position = mvp * finalWorldPosition;
  } else {
    // Outside frustum: place out of screen bounds
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
  }
}
