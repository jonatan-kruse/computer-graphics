#version 410
// This is a Cube mapping shader. It is used to render the skybox. with the textrue "skybox".
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

out VS_OUT {
	vec2 texCoord;
	vec3 normal;
	vec3 vertex;
} vs_out;


void main()
{
	vs_out.texCoord = texCoord;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex, 1.0));
	vs_out.normal = vec3(normal_model_to_world * vec4(normal, 0.0));
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}



