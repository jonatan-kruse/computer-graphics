#version 410 core

in VS_OUT {
    vec3 vertex;   // World-space position from vertex shader
    vec3 normal;   // World-space normal from vertex shader
} fs_in;

out vec4 FragColor;
in vec3 tangents;
in vec3 binormals;

in vec2 normalCoord0;
in vec2 normalCoord1;
in vec2 normalCoord2;


uniform sampler2D normal_map;   // Normal map for the water surface
uniform samplerCube water_texture; 
uniform vec3 light_position;    // Light position in world space
uniform vec3 camera_position;   // Camera position in world space

void main()
{
    mat3 TBN = mat3(normalize(tangents), normalize(binormals), normalize(fs_in.normal));        

    vec3 normal_map_sample = (texture(normal_map,normalCoord0).rgb)*2.0- 1.0;
    normal_map_sample += (texture(normal_map,normalCoord1).rgb)*2.0  - 1.0;
    normal_map_sample += (texture(normal_map,normalCoord2).rgb) *2.0  - 1.0;

    vec3 normal_tangent = normalize(normal_map_sample); 

    vec3 new_normal = TBN * normal_tangent;

    vec3 normal_world = normalize(fs_in.normal);

    vec3 light_dir = normalize(light_position - fs_in.vertex);
    vec3 view_dir = normalize(camera_position - fs_in.vertex);
    vec3 R = reflect(-view_dir, new_normal);
    vec4 reflection = texture(water_texture,R);

    vec3 refraction = refract(-view_dir,new_normal,1.0/1.33);
    vec4 refraction_color = texture(water_texture,refraction);

    float fresnel = 0.02037f + (1 - 0.02037f) * pow(1-dot(view_dir,new_normal),5);


	vec4 colordeep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 colorshallow = vec4(0.0, 0.5, 0.5, 1.0);
    float facing = 1 - max(dot(view_dir,normal_world), 0.0);
    
    FragColor = mix(colordeep,colorshallow, facing)+ (reflection * fresnel) +(refraction_color * (1 - fresnel)) ;

}