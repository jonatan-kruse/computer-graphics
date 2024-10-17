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
	vec3 normal;
	if(use_normal_mapping == 1) {
		vec3 tangetNormal = texture(normalMap, fs_in.texCoord).rgb * 2.0 - 1.0;
		normal = normalize(fs_in.TBN * tangetNormal);
	} else {
		normal = normalize(fs_in.normal);
	}

	vec3 lightDirection = normalize(light_position - fs_in.vertex);
	vec3 viewDirection = normalize(camera_position - fs_in.vertex);
	vec3 reflectedLightDirection = reflect(-lightDirection, normal);

	vec3 ambient_component = ambient * texture(diffuseMap, fs_in.texCoord).rgb;

	float NdotL = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse_component = diffuse * NdotL * texture(diffuseMap, fs_in.texCoord).rgb;

	float RdotV = max(dot(reflectedLightDirection, viewDirection), 0.0);
	float spec_factor = pow(RdotV, shininess);
	vec3 specular_component = specular * spec_factor * texture(specularMap, fs_in.texCoord).rgb;

	vec3 color = ambient_component + diffuse_component + specular_component;

	frag_color = vec4(color, 1.0);
}
