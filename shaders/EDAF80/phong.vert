#version 410
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

out VS_OUT {
	vec2 texCoord;
	vec3 vertex;
	vec3 normal;
	mat3 TBN;
} vs_out;

void main() {
	vec3 T = normalize(mat3(vertex_model_to_world) * tangent);
	vec3 B = normalize(mat3(vertex_model_to_world) * binormal);
	vec3 N = normalize(mat3(vertex_model_to_world) * normal);
	mat3 TBN = mat3(T, B, N);
	vs_out.TBN = TBN;
	vs_out.texCoord = texCoord;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex, 1.0));
	vs_out.normal = normalize(vec3(normal_model_to_world * vec4(normal, 0.0)));
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}
