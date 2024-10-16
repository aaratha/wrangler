#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambient;
uniform vec3 viewPos;

uniform mat4 lightVP;
uniform sampler2D shadowMap;

uniform int shadowMapResolution;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec3 l = -lightDir;

    float NdotL = max(dot(normal, l), 0.0);
    lightDot += lightColor.rgb * NdotL;

    float specCo = 0.0;
    if (NdotL > 0.0)
        specCo = pow(max(0.0, dot(viewD, reflect(-(l), normal))), 16.0); // 16 refers to shine
    specular += specCo;

    finalColor = (texelColor * ((colDiffuse + vec4(specular, 1.0)) * vec4(lightDot, 1.0)));

    // Shadow calculations
    vec4 fragPosLightSpace = lightVP * vec4(fragPosition, 1);
    fragPosLightSpace.xyz /= fragPosLightSpace.w; // Perform the perspective division
    fragPosLightSpace.xyz = (fragPosLightSpace.xyz + 1.0f) / 2.0f; // Transform from [-1, 1] to [0, 1]
    vec2 sampleCoords = fragPosLightSpace.xy;
    float curDepth = fragPosLightSpace.z;

    // Slope-scale depth bias
    float bias = max(0.0002 * (1.0 - dot(normal, l)), 0.00002) + 0.00001;
    int shadowCounter = 0;
    const int numSamples = 9;

    // PCF (percentage-closer filtering)
    vec2 texelSize = vec2(1.0f / float(shadowMapResolution));
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float sampleDepth = texture(shadowMap, sampleCoords + texelSize * vec2(x, y)).r;
            if (curDepth - bias > sampleDepth)
            {
                shadowCounter++;
            }
        }
    }

    // Mix with a soft shadow color instead of full black
    vec4 shadowColor = vec4(0.0, 0.0, 0.0, 1.0); // Adjust this value to control shadow intensity
    float shadowFactor = float(shadowCounter) / float(numSamples);
    finalColor = mix(finalColor, shadowColor, shadowFactor * 0.8); // 0.8 reduces the shadow intensity

    // Add ambient lighting
    finalColor += texelColor * (ambient / 5.0) * colDiffuse; // Increase ambient contribution

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0 / 2.2));
}
