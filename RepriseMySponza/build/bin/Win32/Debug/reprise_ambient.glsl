#version 330

layout(std140) uniform PerDrawUniforms
{
	mat4 projection_view_model_xform;
	mat4 model_xform;
	vec3 diffuse_colour;
	float shininess;
	vec3 specular_colour;
};

uniform sampler2D diffuse_texture;

in vec2 texture_coord_out;
in vec3 N;
in vec3 P;
in vec3 eyeDir;

out vec4 fragment_colour;

void main(void)
{
	vec3 colour = vec3(0.1, 0.1, 0.1) * diffuse_colour;

	//WITH TEXTURES
	fragment_colour = texture(diffuse_texture, texture_coord_out) * vec4(colour, 1);
}

