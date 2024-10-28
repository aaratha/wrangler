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

// Output to the fragment shader
out vec2 fragTexCoord;
out vec3 fragPosition;

void main() {
  fragTexCoord = vertexTexCoord;
  vec3 position = vertexPosition;

  // Extract wind parameters
  float windStrength = windParams.x;
  float windFrequency = windParams.y;
  float windSpeed = windParams.z;
  float time = windParams.w;

  vec4 worldPosition = instanceTransform * vec4(position, 1.0);
  // Frustum culling: calculate the vector from camera to instance position
  vec3 toBlade = worldPosition.xyz - cameraPos;
  float distance = length(toBlade);

  // Calculate the angle between the camera direction and the to-blade vector
  float angle = acos(dot(normalize(toBlade), cameraDir));

  // Perform frustum culling by checking if the angle is within half of the
  // field of view
  if (angle < (fovy) * 0.01) {
    // Calculate wind oscillation based on blade position
    if (angle < fovy * 0.005) {
      vec3 basePos = vec3(instanceTransform[3][0], instanceTransform[3][1],
                          instanceTransform[3][2]);
      float oscillation =
          sin(windFrequency * basePos.x + windSpeed * time) *
          cos(windFrequency * basePos.z + windSpeed * time * 0.7);
      float windDisplacement = oscillation * windStrength * position.y;

      // Apply wind displacement directly to the vertex position
      position.x += windDisplacement;
      position.z += windDisplacement * 0.5;
    }

    // Calculate the world position of the blade
    vec4 worldPosition = instanceTransform * vec4(position, 1.0);
    fragPosition = worldPosition.xyz;

    // Inside frustum: project to screen space
    gl_Position = mvp * worldPosition;
  } else {
    // Outside frustum: place out of screen bounds to skip rendering
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
  }
}
