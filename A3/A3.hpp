#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "GeometryNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <stack>

#define MAXSTACK 50

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};


class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();

	void updateShaderUniforms(const ShaderProgram & shader, const GeometryNode & node, const glm::mat4 & viewMatrix);

	bool Undo(SceneNode &m_rootNode);
	bool Redo(SceneNode &m_rootNode);
	void resetJoint(SceneNode &m_rootNode);

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	// Flags related to options
	bool circle, z_buffer, backcull, frontcull;
	int mode;

	// Fields related to dragging
	bool left, middle, right;
	GLfloat pre_xpos, pre_ypos;
	GLfloat cur_xpos, cur_ypos;

	glm::vec4 origin;
	bool do_picking;
	
	GLfloat head_angle;
	glm::mat4 m_joint[50];
	std::stack<glm::mat4>undo_s[50];
	std::stack<double>undo_init_x[50];
	std::stack<double>undo_init_y[50];
	std::stack<double>undo_head_s;

	std::stack<glm::mat4>redo_s[50];
	std::stack<double>redo_init_x[50];
	std::stack<double>redo_init_y[50];
	std::stack<double>redo_head_s;

	bool allow_undo, allow_redo;
};
