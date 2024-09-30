#version 410

uniform vec3 light_position;

in VS_OUT {
	vec3 texCoord;
} fs_in;

out vec4 frag_color;
uniform samplerCube skybox;

void main()
{
	vec3 tex = fs_in.texCoord;
	frag_color = texture(skybox, tex);
}
