#version 410 core

layout (location = 0) in vec3 vertex;  // Vertex position
layout (location = 1) in vec3 normal;  // Normal vector
layout (location = 2) in vec3 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;    // Model matrix 
uniform mat4 normal_model_to_world;    // Normal matrix 
uniform mat4 vertex_world_to_clip;     // Projection * View matrix 
uniform float elapsed_time;            // Time uniform for wave animation
uniform float wave_speed;              // Speed of wave animation
uniform float wave_amplitude;          // Amplitude multiplier for waves

out vec2 normalCoord0;
out vec2 normalCoord1;
out vec2 normalCoord2;
out vec3 tangents;
out vec3 binormals;

out VS_OUT {
    vec3 vertex;   // Transformed vertex 
    vec3 normal;   // Transformed normal 
} vs_out;

// Function to generate a wave displacement
float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time, float speed)
{
    return amplitude * pow(sin((position.x * direction.x + position.y * direction.y) * frequency + phase * time * speed) * 0.5 + 0.5, sharpness);
}

// Function to calculate wave derivatives for normal calculation
float derivative(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time, float speed)
{
    return 0.5 * sharpness * frequency * amplitude *
           pow(sin((position.x * direction.x + position.y * direction.y) * frequency + phase * time * speed) * 0.5 + 0.5, sharpness - 1) *
           cos((position.x * direction.x + position.y * direction.y) * frequency + time * speed * phase);
}

void main()
{
    vec2 texScale = vec2(8, 4);
    float normalTime = mod(elapsed_time, 100.0);
    vec2 normalSpeed = vec2(-0.05, 0.0);

    normalCoord0.xy = texCoord.xz * texScale + normalTime * normalSpeed;
    normalCoord1.xy = texCoord.xz * texScale * 2 + normalTime * normalSpeed * 4;
    normalCoord2.xy = texCoord.xz * texScale * 4 + normalTime * normalSpeed * 8;

    vec3 displaced_vertex = vertex; 
    displaced_vertex.y += wave(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed);
    displaced_vertex.y += wave(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed);

    vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));

    vs_out.normal = vec3(derivative(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed) * -1, 1,
                         derivative(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed) * 0);  
    vs_out.normal += vec3(derivative(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed) * -0.7, 0,
                          derivative(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed) * 0.7);

    tangents = vec3(1, derivative(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed) * -1 + 
                      derivative(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed) * 0, 0);

    binormals = vec3(0, derivative(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed) * -0.7 + 
                      derivative(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed) * 0.7, 1);

    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vs_out.vertex, 1.0);
}
