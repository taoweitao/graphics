#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	void initGrid();
	void updateCube(float x, float y, float z);
	void reset();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Matrices controlling the camera and projection and model.
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 model;

	glm::vec3 colour[8];
	int current_col;

	// Fields related to cube geometry.
	GLuint m_cube_vao;
	GLuint m_cube_vbo;

	// Position related to active cell.
	GLuint m_active_x;
	GLuint m_active_z;

	// Fields related to cells.
	glm::vec3 m_color[18][18];
	GLuint color_type[18][18];
	GLuint num[18][18];

	// Fields related to copy height.
	bool is_copy;
	GLuint num_copy;
	glm::vec3 color_copy;

	// Counter related to indicator.
	GLuint indicator;
	glm::vec3 indicator_color;

	// Fields related to dragging.
	bool dragging;
	GLfloat pre_xpos, pre_ypos;
	GLfloat cur_xpos, cur_ypos;

	// Bounds related to scaling.
	GLfloat bound;
};
