#version 330
// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in mat4 instanceTransform;
// Input uniform values
uniform mat4 mvp;
uniform vec4 windParams;  // x: strength, y: frequency, z: speed, w: time
// Output vertex attributes
out vec2 fragTexCoord;
out vec3 fragPosition;
void main()
{
    fragTexCoord = vertexTexCoord;
    vec3 position = vertexPosition;

    float windStrength = windParams.x;
    float windFrequency = windParams.y;
    float windSpeed = windParams.z;
    float time = windParams.w;

    // Use instance position for base wind calculation
    vec3 basePos = vec3(instanceTransform[3][0], instanceTransform[3][1], instanceTransform[3][2]);
    float baseWindAngle = windStrength *
        sin(windFrequency * basePos.x + windSpeed * time) *
        cos(windFrequency * basePos.z + windSpeed * time * 0.7);

    // Scale displacement by height but use base wind angle
    float heightFactor = position.y;
    float windDisplacement = baseWindAngle * heightFactor;

    // Apply displacement while maintaining blade shape
    position.x += windDisplacement;
    position.z += windDisplacement * 0.5;

    vec4 worldPosition = instanceTransform * vec4(position, 1.0);
    fragPosition = worldPosition.xyz;
    gl_Position = mvp * worldPosition;
}
