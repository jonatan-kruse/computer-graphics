#version 410

uniform vec3 light_position;

in VS_OUT {
	vec2 texCoord;
	vec3 normal;
	vec3 vertex;
} fs_in;

out vec4 frag_color;
uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D normal;

void main()
{
	vec3 L = normalize(light_position - fs_in.vertex);
	frag_color = texture(diffuse, fs_in.texCoord) * clamp(dot(normalize(fs_in.normal), L), 0.0, 1.0);

}
