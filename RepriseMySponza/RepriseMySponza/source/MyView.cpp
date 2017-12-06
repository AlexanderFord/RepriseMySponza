#include "MyView.hpp"

#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const sponza::Context * scene)
{
    scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);
	GLint compile_status = 0;

	/*Shaders are generated here*/

	///VERTEX SHADER GENERATION
	//Vertex Shader
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_string = tygra::createStringFromFile("resource:///reprise_vs.glsl");
	const char *vertex_shader_code = vertex_shader_string.c_str();

	glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE) 
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	///FRAGMENT SHADER GENERATION
	//Ambient shader
	GLuint ambient_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string ambient_shader_string = tygra::createStringFromFile("resource:///reprise_ambient.glsl");
	const char *ambient_shader_code = ambient_shader_string.c_str();

	glShaderSource(ambient_shader, 1, (const GLchar **)&ambient_shader_code, NULL);
	glCompileShader(ambient_shader);
	glGetShaderiv(ambient_shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(ambient_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	//Phong Shader
	GLuint phong_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string phong_shader_string = tygra::createStringFromFile("resource:///reprise_phong.glsl");
	const char *phong_shader_code = phong_shader_string.c_str();

	glShaderSource(phong_shader, 1, (const GLchar **)&phong_shader_code, NULL);
	glCompileShader(phong_shader);
	glGetShaderiv(phong_shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(phong_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}
	
	//Create the program used for the vertex shader + ambient shader
	reprise_my_sponza_ = glCreateProgram();

	//Attach the data to the program
	glAttachShader(reprise_my_sponza_, vertex_shader);
	glBindAttribLocation(reprise_my_sponza_, 0, "vertex_position");
	glBindAttribLocation(reprise_my_sponza_, 1, "vertex_normal");
	glBindAttribLocation(reprise_my_sponza_, 2, "texture_coord_in");
	glDeleteShader(vertex_shader);
	glAttachShader(reprise_my_sponza_, ambient_shader);
	glBindFragDataLocation(reprise_my_sponza_, 0, "fragment_colour");
	glDeleteShader(ambient_shader);

	//Link the program
	glLinkProgram(reprise_my_sponza_);

	GLint link_status = 0;
	glGetProgramiv(reprise_my_sponza_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(reprise_my_sponza_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	//Create the phong shader program
	phong_shader_prog_ = glCreateProgram();

	//Attach the data to the program
	glAttachShader(phong_shader_prog_, vertex_shader);
	glBindAttribLocation(phong_shader_prog_, 0, "vertex_position");
	glBindAttribLocation(phong_shader_prog_, 1, "vertex_normal");
	glBindAttribLocation(phong_shader_prog_, 2, "texture_coord_in");
	glDeleteShader(vertex_shader);
	glAttachShader(phong_shader_prog_, phong_shader);
	glBindFragDataLocation(phong_shader_prog_, 0, "fragment_colour");
	glDeleteShader(phong_shader);

	//Link the program
	glLinkProgram(phong_shader_prog_);

	link_status = 0;
	glGetProgramiv(phong_shader_prog_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(phong_shader_prog_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	//Retrieves the meshes from a data file
	sponza::GeometryBuilder sponza;

	const auto& scene_meshes = sponza.getAllMeshes();

	for (const auto& scene_mesh : scene_meshes)
	{
		MeshGL &newMesh_ = sponza_mesh_[scene_mesh.getId()];

		const auto& positions = scene_mesh.getPositionArray();
		const auto& normals = scene_mesh.getNormalArray();
		const auto& elements = scene_mesh.getElementArray();
		const auto& texcoords = scene_mesh.getTextureCoordinateArray();

		/*Various buffers are created here*/

		///UNIFORM BUFFER OBJECTS
		//PerDrawUBO
		glGenBuffers(1, &per_draw_ubo_);
		glBindBuffer(GL_UNIFORM_BUFFER, per_draw_ubo_);
		glBufferData(GL_UNIFORM_BUFFER,
			sizeof(PerDrawUniforms),
			nullptr,
			GL_STREAM_DRAW);

		//PerFrameUBO
		glGenBuffers(1, &per_frame_ubo_);
		glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo_);
		glBufferData(GL_UNIFORM_BUFFER,
			sizeof(PerFrameUniforms),
			nullptr,
			GL_STREAM_DRAW);

		///VERTEX BUFFER OBJECTS
		//Position VBO
		glGenBuffers(1, &newMesh_.position_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, newMesh_.position_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			positions.size() * sizeof(glm::vec3),						//Size of the data in bytes
			positions.data(),											//Pointer to the data
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Normal VBO
		glGenBuffers(1, &newMesh_.normal_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, newMesh_.normal_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			normals.size() * sizeof(glm::vec3),							//Size of the data in bytes
			normals.data(),												//Pointer to the data
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Texture Coordinates
		glGenBuffers(1, &newMesh_.texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, newMesh_.texcoord);
		glBufferData(GL_ARRAY_BUFFER,
			texcoords.size() * sizeof(glm::vec2),						//Size of the data in bytes
			texcoords.data(),											//Pointer to the data
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//Element VBO
		glGenBuffers(1, &newMesh_.element_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh_.element_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			elements.size() * sizeof(unsigned int),						//Size of the data in bytes
			elements.data(),											//Pointer to the data
			GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		newMesh_.element_count = elements.size();

		//VAO
		glGenVertexArrays(1, &newMesh_.vao);
		glBindVertexArray(newMesh_.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh_.element_vbo);

		//Positions
		glBindBuffer(GL_ARRAY_BUFFER, newMesh_.position_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

		//Normals
		glBindBuffer(GL_ARRAY_BUFFER, newMesh_.normal_vbo);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));

		//Texture Coordinates
		glBindBuffer(GL_ARRAY_BUFFER, newMesh_.texcoord);
		if (texcoords.size() <= 0)
		{
			glDisableVertexAttribArray(2);
		}
		else
		{
			glEnableVertexAttribArray(2);
		}

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec2), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//USE THIS FOR ADDING TEXTURES
	const auto& scene_materials = scene_->getAllMaterials();

	for (const auto& material : scene_materials)
	{
		tygra::Image texture_image = tygra::createImageFromPngFile("resource:///hex.png");
		if (texture_image.doesContainData())
		{
			glGenTextures(1, &diffuse_texture);
			glBindTexture(GL_TEXTURE_2D, diffuse_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				texture_image.width(),
				texture_image.height(),
				0,
				pixel_formats[texture_image.componentsPerPixel()],
				texture_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
				texture_image.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	//Delete the shading programs upon exit
	glDeleteProgram(reprise_my_sponza_);
	glDeleteProgram(phong_shader_prog_);

	//Delete the buffers upon exit
	sponza::GeometryBuilder builder;

	const auto& scene_meshes = builder.getAllMeshes();

	for (const auto& mesh_ : scene_meshes)
	{
		const MeshGL& mesh = sponza_mesh_[mesh_.getId()];

		glDeleteBuffers(1, &per_draw_ubo_);
		glDeleteBuffers(1, &per_frame_ubo_);
		glDeleteBuffers(1, &mesh.position_vbo);
		glDeleteBuffers(1, &mesh.element_vbo);
		glDeleteBuffers(1, &mesh.normal_vbo);
		glDeleteBuffers(1, &mesh.texcoord);
		glDeleteBuffers(1, &mesh.colour_vbo);
		glDeleteBuffers(1, &mesh.vao);
	}

	//Delete the textures upon exit
	for (const auto& textureDelete : diffuseMap)
	{
		glDeleteBuffers(1, &textureDelete.second);
	}
}

/*Function used for drawing the ambient shader to the scene*/
void MyView::ambientRender()
{
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	//Set up the view xform
	glm::vec3 eye = (const glm::vec3&)scene_->getCamera().getPosition();
	glm::vec3 direction = (const glm::vec3&)scene_->getCamera().getDirection();

	GLuint view_xform_id = glGetUniformLocation(reprise_my_sponza_, "view_xform");
	glm::mat4 view_xform = glm::lookAt(eye, eye + direction, (const glm::vec3&)scene_->getUpDirection());
	glUniformMatrix4fv(view_xform_id, 1, GL_FALSE, glm::value_ptr(view_xform));

	//Set up the projection xform
	GLuint projection_xform_id = glGetUniformLocation(reprise_my_sponza_, "projection_xform");
	glm::mat4 projection_xform = glm::perspective(glm::radians(75.0f), aspect_ratio, 1.f, 1000.f);
	glUniformMatrix4fv(projection_xform_id, 1, GL_FALSE, glm::value_ptr(projection_xform));

	//Set up the camera position
	GLuint camera_position_id = glGetUniformLocation(reprise_my_sponza_, "camera_position");
	glUniform3fv(camera_position_id, 1, glm::value_ptr(eye));

	//Set up the lighting
	const auto lights = scene_->getAllPointLights();
	int i = 0;

	for (const auto& light : lights)
	{
		//Position
		glm::vec3 position = (const glm::vec3 &)light.getPosition();
		per_frame_uniforms.light[i].position = position;

		//Range
		float range = (const float &)light.getRange();
		per_frame_uniforms.light[i].range = range;

		//Intensity
		glm::vec3 intensity = (const glm::vec3 &)light.getIntensity();
		per_frame_uniforms.light[i].intensity = intensity;

		i++;
	}
	
	for (const auto& instance : scene_->getAllInstances())
	{
		const MeshGL& mesh_ = sponza_mesh_[instance.getMeshId()];

		const auto& material = scene_->getMaterialById(instance.getMaterialId());

		//Set up the model xform
		GLuint model_xform_id = glGetUniformLocation(reprise_my_sponza_, "model_xform");
		glm::mat4 model_xform = glm::mat4((const glm::mat4x3&)instance.getTransformationMatrix());
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

		//Set up the diffuse colour
		GLuint diffuse_colour_id = glGetUniformLocation(reprise_my_sponza_, "diffuse_colour");
		glm::vec3 diffuse_colour = (glm::vec3 &)material.getDiffuseColour();
		glUniform3fv(diffuse_colour_id, 1, glm::value_ptr(diffuse_colour));

		//Set up shininess
		GLuint shininess_id = glGetUniformLocation(reprise_my_sponza_, "shininess");
		float shininess = material.getShininess();
		glUniform1fv(shininess_id, 1, &shininess);

		//Set up specular colour
		GLuint specular_colour_id = glGetUniformLocation(reprise_my_sponza_, "specular_colour");
		glm::vec3 specular_colour = (glm::vec3 &)material.getSpecularColour();
		glUniform3fv(specular_colour_id, 1, glm::value_ptr(specular_colour));

		//Set up the diffuse texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		GLuint diffuse_texture_id = glGetUniformLocation(reprise_my_sponza_, "diffuse_texture");
		glUniform1i(diffuse_texture_id, 0);

		//Assign uniforms
		per_frame_uniforms.view_xform = view_xform;
		per_frame_uniforms.projection_xform = projection_xform;
		per_frame_uniforms.camera_position = eye;

		per_draw_uniforms.model_xform = model_xform;
		per_draw_uniforms.diffuse_colour = diffuse_colour;
		per_draw_uniforms.shininess = shininess;
		per_draw_uniforms.specular_colour = specular_colour;

		per_draw_uniforms.projection_view_model_xform = projection_xform * view_xform * model_xform;

		/*TODO: Figure out why uniforms are being set to the maximum buffer block index*/
		//Fill the uniform buffers with data and bind them to the appropriate block_index
		glBindBuffer(GL_UNIFORM_BUFFER, per_draw_ubo_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(per_draw_uniforms), &per_draw_uniforms);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, per_draw_ubo_);
		glUniformBlockBinding(reprise_my_sponza_, glGetUniformBlockIndex(reprise_my_sponza_, "PerDrawUniforms"), 0);

		glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(per_frame_uniforms), &per_frame_uniforms);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, per_frame_ubo_);
		glUniformBlockBinding(reprise_my_sponza_, glGetUniformBlockIndex(reprise_my_sponza_, "PerFrameUniforms"), 1);

		//Specify the VAO to use
		glBindVertexArray(mesh_.vao);

		//Draw
		glDrawElements(GL_TRIANGLES, mesh_.element_count, GL_UNSIGNED_INT, 0);
	}
}

/*Function used for drawing the phong shader to the scene*/
void MyView::phongRender()
{
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	//Set up the view xform
	glm::vec3 eye = (const glm::vec3&)scene_->getCamera().getPosition();
	glm::vec3 direction = (const glm::vec3&)scene_->getCamera().getDirection();

	GLuint view_xform_id = glGetUniformLocation(phong_shader_prog_, "view_xform");
	glm::mat4 view_xform = glm::lookAt(eye, eye + direction, (const glm::vec3&)scene_->getUpDirection());
	glUniformMatrix4fv(view_xform_id, 1, GL_FALSE, glm::value_ptr(view_xform));

	//Set up the projection xform
	GLuint projection_xform_id = glGetUniformLocation(phong_shader_prog_, "projection_xform");
	glm::mat4 projection_xform = glm::perspective(glm::radians(75.0f), aspect_ratio, 1.f, 1000.f);
	glUniformMatrix4fv(projection_xform_id, 1, GL_FALSE, glm::value_ptr(projection_xform));

	//Set up the camera position
	GLuint camera_position_id = glGetUniformLocation(phong_shader_prog_, "camera_position");
	glUniform3fv(camera_position_id, 1, glm::value_ptr(eye));

	//Set up the lighting
	const auto lights = scene_->getAllPointLights();
	int i = 0;

	for (const auto& light : lights)
	{
		//Position
		glm::vec3 position = (const glm::vec3 &)light.getPosition();
		per_frame_uniforms.light[i].position = position;

		//Range
		float range = (const float &)light.getRange();
		per_frame_uniforms.light[i].range = range;

		//Intensity
		glm::vec3 intensity = (const glm::vec3 &)light.getIntensity();
		per_frame_uniforms.light[i].intensity = intensity;

		i++;
	}

	for (const auto& instance : scene_->getAllInstances())
	{
		const MeshGL& mesh_ = sponza_mesh_[instance.getMeshId()];

		const auto& material = scene_->getMaterialById(instance.getMaterialId());

		//Set up the model xform
		GLuint model_xform_id = glGetUniformLocation(phong_shader_prog_, "model_xform");
		glm::mat4 model_xform = glm::mat4((const glm::mat4x3&)instance.getTransformationMatrix());
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

		//Set up the diffuse colour
		GLuint diffuse_colour_id = glGetUniformLocation(phong_shader_prog_, "diffuse_colour");
		glm::vec3 diffuse_colour = (glm::vec3 &)material.getDiffuseColour();
		glUniform3fv(diffuse_colour_id, 1, glm::value_ptr(diffuse_colour));

		//Set up shininess
		GLuint shininess_id = glGetUniformLocation(phong_shader_prog_, "shininess");
		float shininess = material.getShininess();
		glUniform1fv(shininess_id, 1, &shininess);

		//Set up specular colour
		GLuint specular_colour_id = glGetUniformLocation(phong_shader_prog_, "specular_colour");
		glm::vec3 specular_colour = (glm::vec3 &)material.getSpecularColour();
		glUniform3fv(specular_colour_id, 1, glm::value_ptr(specular_colour));

		//Set up the diffuse texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		GLuint diffuse_texture_id = glGetUniformLocation(phong_shader_prog_, "diffuse_texture");
		glUniform1i(diffuse_texture_id, 0);

		//Assign uniforms
		per_frame_uniforms.view_xform = view_xform;
		per_frame_uniforms.projection_xform = projection_xform;
		per_frame_uniforms.camera_position = eye;

		per_draw_uniforms.model_xform = model_xform;
		per_draw_uniforms.diffuse_colour = diffuse_colour;
		per_draw_uniforms.shininess = shininess;
		per_draw_uniforms.specular_colour = specular_colour;

		per_draw_uniforms.projection_view_model_xform = projection_xform * view_xform * model_xform;

		/*TODO: Figure out why uniforms are being set to the maximum buffer block index*/
		//Fill the uniform buffers with data and bind them to the appropriate block_index
		glBindBuffer(GL_UNIFORM_BUFFER, per_draw_ubo_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(per_draw_uniforms), &per_draw_uniforms);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, per_draw_ubo_);
		glUniformBlockBinding(phong_shader_prog_, glGetUniformBlockIndex(phong_shader_prog_, "PerDrawUniforms"), 2);

		glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(per_frame_uniforms), &per_frame_uniforms);
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, per_frame_ubo_);
		glUniformBlockBinding(phong_shader_prog_, glGetUniformBlockIndex(phong_shader_prog_, "PerFrameUniforms"), 3);

		//Specify the VAO to use
		glBindVertexArray(mesh_.vao);

		//Draw
		glDrawElements(GL_TRIANGLES, mesh_.element_count, GL_UNSIGNED_INT, 0);
	}
}

void MyView::windowViewRender(tygra::Window * window)
{
    assert(scene_ != nullptr);

	glEnable(GL_CULL_FACE);

	///MULTIPASS RENDERING
	//Set up the depth test for drawing the ambient shader
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(reprise_my_sponza_);
	ambientRender();

	//Set up the depth test for drawing the phong shader
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glUseProgram(phong_shader_prog_);
	phongRender();

	glDisable(GL_BLEND);
}
