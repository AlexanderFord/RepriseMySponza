#version 330

layout(std140) uniform PerDrawUniforms
{
	mat4 projection_view_model_xform;
	mat4 model_xform;
	vec3 diffuse_colour;
	float shininess;
	vec3 specular_colour;
};

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 texture_coord_in;

out vec2 texture_coord_out;
out vec3 P;
out vec3 N;
out vec3 eyeDir;

void main(void)
{
	texture_coord_out = texture_coord_in;

	// Move the position into world space.
	P = vec3((model_xform*vec4((vertex_position), 1.0)).xyz);
	N = vec3((model_xform*vec4((normalize(vertex_normal)), 0.0)).xyz);

	gl_Position = (projection_view_model_xform) * vec4(vertex_position, 1.0);
}