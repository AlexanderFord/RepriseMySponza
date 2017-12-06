#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

class MyView : public tygra::WindowViewDelegate
{
public:

    MyView();

    ~MyView();

    void setScene(const sponza::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;

    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;

    void windowViewRender(tygra::Window * window) override;

	void ambientRender();
	void phongRender();

	struct lights
	{
		glm::vec3 position;
		float range;
		glm::vec3 intensity;
		float pad0;
	};

	//Uniforms to be executed per draw
	struct PerDrawUniforms
	{
		glm::mat4 projection_view_model_xform;
		glm::mat4 model_xform;
		glm::vec3 diffuse_colour;
		float shininess;
		glm::vec3 specular_colour;
		float pad0;
	};

	//Uniforms to be executed per frame
	struct PerFrameUniforms
	{
		glm::mat4 view_xform;
		glm::mat4 projection_xform;
		glm::vec3 camera_position;
		float pad0;
		lights light[22];
	};

	PerDrawUniforms per_draw_uniforms;
	PerFrameUniforms per_frame_uniforms;

	GLuint reprise_my_sponza_;
	GLuint phong_shader_prog_;

	GLuint per_draw_ubo_;
	GLuint per_frame_ubo_;

	GLuint diffuse_texture;
	GLuint sampler;

	//Decleration for multiple VBO
	struct MeshGL
	{
		GLuint position_vbo; // VertexBufferObject for the vertex positions
		GLuint element_vbo;  // VertexBufferObject for the elements (indices)
		GLuint vao;			 // VertexArrayObject for the shape's vertex array settings
		GLuint colour_vbo;   // VertexBufferObject
		GLuint normal_vbo;   // NormalBufferObject for the normals
		GLuint texcoord;	 // Texture coordinates
		int element_count;   // Needed for when we draw using the vertex arrays

		MeshGL() : position_vbo(0),
			element_vbo(0),
			vao(0),
			colour_vbo(0),
			normal_vbo(0),
			texcoord(0),
			element_count(0) {}
	};

	std::map<sponza::MeshId, MeshGL> sponza_mesh_;
	std::unordered_map<std::string, GLuint> diffuseMap;

    const sponza::Context * scene_;

};
