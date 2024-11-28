#version 410

struct ViewProjTransforms {
	mat4 view_projection;
	mat4 view_projection_inverse;
};

layout(std140) uniform CameraViewProjTransforms {
	ViewProjTransforms camera;
};

layout(std140) uniform LightViewProjTransforms {
	ViewProjTransforms lights[4];
};

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout(location = 0) out vec4 light_diffuse_contribution;
layout(location = 1) out vec4 light_specular_contribution;

void main() {
	vec2 shadowmap_texel_size = 1.0 / textureSize(shadow_texture, 0);
	vec2 texcoord = gl_FragCoord.xy * inverse_screen_resolution;
	vec3 normal = texture(normal_texture, texcoord).xyz * 2.0 - 1.0;
	vec4 position = vec4((2 * texcoord - 1), texture(depth_texture, texcoord).x * 2 - 1, 1);
	vec4 world_position = camera.view_projection_inverse * position;
	world_position /= world_position.w;

	vec3 light_dir = normalize(light_position - world_position.xyz);
	float ndotl = dot(normal, light_dir);
	float light_distance = length(light_position - world_position.xyz);
	float light_attenuation = 1.0 / (1 + (light_distance * light_distance * 0.000003));
	vec3 n_light_direction = normalize(light_direction);
	vec3 n_light_dir = normalize(light_dir);
	float light_angle = dot(n_light_direction, -n_light_dir);
	float angle_falloff = (light_angle - cos(light_angle_falloff)) / (1 - cos(light_angle_falloff));
	vec3 light = light_color * light_attenuation * angle_falloff;

	vec3 view_dir = normalize(camera_position - world_position.xyz);
	vec3 reflect_dir = reflect(-light_dir, normal);

	light_diffuse_contribution = vec4(light * max(ndotl, 0.0), 1.0);
	light_specular_contribution = vec4(light * max(dot(view_dir, reflect_dir), 0.0), 1.0);
	// light_diffuse_contribution = vec4(angle_falloff);
	// light_specular_contribution = vec4(angle_falloff);
	
}
