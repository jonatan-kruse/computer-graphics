#version 410 core

layout(location = 0) in vec3 vertex;  // Vertex position
layout(location = 1) in vec3 normal;  // Normal vector
layout(location = 2) in vec3 texCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

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

float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time, float speed) {
    return amplitude * pow(sin(dot(position, direction) * frequency + phase * time * speed) * 0.5 + 0.5, sharpness);
}

// derivative function to calculate gradient vector
vec2 derivative(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time, float speed) {
    float S = dot(position, direction) * frequency + phase * time * speed;
    float wave_value = sin(S) * 0.5 + 0.5;
    float common_factor = 0.5 * sharpness * frequency * amplitude * pow(wave_value, sharpness - 1.0) * cos(S);
    return common_factor * direction;
}

void main() {
    vec2 texScale = vec2(8, 4);
    float normalTime = mod(elapsed_time, 100.0);
    vec2 normalSpeed = vec2(-0.05, 0.0);

    normalCoord0.xy = texCoord.xz * texScale + normalTime * normalSpeed;
    normalCoord1.xy = texCoord.xz * texScale * 2.0 + normalTime * normalSpeed * 4.0;
    normalCoord2.xy = texCoord.xz * texScale * 4.0 + normalTime * normalSpeed * 8.0;

    vec3 displaced_vertex = vertex;
    displaced_vertex.y += wave(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed);
    displaced_vertex.y += wave(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed);

    vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));

    vec2 gradG1 = derivative(vertex.xz, vec2(-1.0, 0.0), 1.0 * wave_amplitude, 0.2, 0.5, 2.0, elapsed_time, wave_speed);
    vec2 gradG2 = derivative(vertex.xz, vec2(-0.70, 0.7), 0.5 * wave_amplitude, 0.4, 1.3, 2.0, elapsed_time, wave_speed);

    vec2 gradH = gradG1 + gradG2;

    tangents = normalize(vec3(1.0, gradH.x, 0.0));
    binormals = normalize(vec3(0.0, gradH.y, 1.0));

    vs_out.normal = normalize(-cross(tangents, binormals));

    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0);
}
