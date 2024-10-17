#version 410
// This is a Cube mapping shader. It is used to render the skybox. with the textrue "skybox".
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform vec3 camera_position;

out VS_OUT {
	vec3 texCoord;
} vs_out;


void main()
{
	vs_out.texCoord = vertex;
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}



