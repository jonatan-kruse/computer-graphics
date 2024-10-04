#version 410

uniform vec3 light_position;
uniform vec3 camera_position;
uniform int use_normal_mapping;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

in VS_OUT {
	vec2 texCoord;
	vec3 normal;
	vec3 vertex;
	mat3 TBN;
} fs_in;

out vec4 frag_color;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

void main() {
	vec3 N;
	if(use_normal_mapping == 1) {
		vec3 tangetNormal = texture(normalMap, fs_in.texCoord).rgb * 2.0 - 1.0;
		N = normalize(fs_in.TBN * tangetNormal); // Using interpolated normal for now
	} else {
		N = normalize(fs_in.normal);
	}

	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 V = normalize(camera_position - fs_in.vertex);
	vec3 R = reflect(-L, N);

    // Ambient component
	vec3 ambient_component = ambient * texture(diffuseMap, fs_in.texCoord).rgb;

    // Diffuse component
	float NdotL = max(dot(N, L), 0.0);
	vec3 diffuse_component = diffuse * NdotL * texture(diffuseMap, fs_in.texCoord).rgb;

    // Specular component
	float RdotV = max(dot(R, V), 0.0);
	float spec_factor = pow(RdotV, shininess);
	vec3 specular_component = specular * spec_factor * texture(specularMap, fs_in.texCoord).rgb;

	vec3 final_color = ambient_component + diffuse_component + specular_component;

	frag_color = vec4(final_color, 1.0);
}
