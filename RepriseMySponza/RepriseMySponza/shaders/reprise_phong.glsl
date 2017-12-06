#version 330

struct lights
{
	vec3 position;
	float range;
	vec3 intensity;
};

layout(std140) uniform PerFrameUniforms
{	
	mat4 view_xform;
	mat4 projection_xform;
	vec3 camera_position;
	lights light[22];
};

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
	vec3 colour = vec3(0.f, 0.f, 0.f);

	for (int i = 0; i < 22; i++)
	{
		//Get the light position/distance for the xyz
		float xDist = light[i].position.x - P.x;
		float yDist = light[i].position.y - P.y;
		float zDist = light[i].position.z - P.z;
		float distance = (xDist * xDist) + (yDist * yDist) + (zDist * zDist);

		//Intensity of the light
		float attenuation_coefficient = 0.001f;
		float attenuation = 1.0f / (1.0f + attenuation_coefficient * distance);

		vec3 L = normalize(light[i].position - P);
		float lightPower = light[i].range / 50;
		float lightShininess = shininess / 50;
		vec3 lightColour = light[i].intensity;
		float cosTheta = clamp(dot(N, L), 0, 1);

		//Eye vector
		vec3 E = normalize(eyeDir * mat3(model_xform));

		//Direction light is reflected
		vec3 R = reflect(-L, N);

		//Cosine of the angle between the eye vector and the reflect vector						
		float cosAlpha = clamp(dot(E, R), 0, 1);
		colour += diffuse_colour * lightColour * lightPower * cosTheta * attenuation +
				specular_colour * lightShininess * lightPower * pow(cosAlpha, 5) * attenuation;
	}
	fragment_colour = texture(diffuse_texture, texture_coord_out) * vec4(colour, 1);
}

