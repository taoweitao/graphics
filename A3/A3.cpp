#include "A3.hpp"
#include "scene_lua.hpp"
#include "trackball.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

void printMat4(const glm::mat4 matrix) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++)
			std::cout << matrix[i][j] << " ";
		std::cout << "\n";
	}
}

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */

void initOriginMaterial(SceneNode& root) {
	if (root.m_nodeType == NodeType::GeometryNode)
	{
		GeometryNode* node = static_cast<GeometryNode*>(&root);
		node->origin = node->material;
	}

	for(SceneNode* node: root.children)
		initOriginMaterial(*node);
}

void A3::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
	
	circle = false;
	z_buffer = true;
	backcull = false;
	frontcull = false;
	mode = 1;

	left = false;
	middle = false;
	right = false;

	origin = m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f);
	do_picking = false;

	initOriginMaterial(*m_rootNode);
	head_angle = 0;

	undo_head_s.push(0);
	for(int i = 0; i < MAXSTACK; i++) {
		undo_s[i].push(mat4());
		undo_init_x[i].push(0);
		undo_init_y[i].push(0);
	}

	allow_undo = true;
	allow_redo = true;
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i(location, do_picking ? 1 : 0);

		if(!do_picking) {
			//-- Set LightSource uniform for the scene:
			{
				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(m_light.position));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(0.05f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
SceneNode* search(SceneNode& root, unsigned int id) {
	if(root.m_nodeId == id)
		return &root;
	
	SceneNode *ans;
	for(SceneNode* node: root.children) {
		if(ans = search(*node, id))
			return ans;
	}
	
	return NULL;
}

void rotateAll(SceneNode& root, glm::mat4 rotate, glm::mat4 recovery) {
	root.trans = recovery*rotate*glm::inverse(recovery)*root.trans;
	for(SceneNode* node: root.children) {
		rotateAll(*node, rotate, recovery);
	}
}

void resetPosition(SceneNode &root, glm::mat4 translate_mat) {
	root.trans = translate_mat*root.trans;

	for(SceneNode *node: root.children)
		resetPosition(*node, translate_mat);
}

void resetOrientation(SceneNode &root, glm::mat4 rot_mat) {
	root.trans = rot_mat*root.trans;

	for(SceneNode *node: root.children)
		resetOrientation(*node, rot_mat);
}

bool A3::Undo(SceneNode &m_rootNode) {
	if(undo_head_s.size() == 1)
		return false;

	int m = 0;
	for(int i = 0; i < m_rootNode.totalSceneNodes(); i++) {
		SceneNode *node = search(m_rootNode, i);
		if(node->m_name == "head") {
			if(undo_head_s.size() > 1) {
				GLfloat current = undo_head_s.top();
				redo_head_s.push(current);
				undo_head_s.pop();
				glm::mat4 rot = glm::rotate(mat4(), GLfloat(undo_head_s.top()) - current, vec3(0.0f, 1.0f, 0.0f));
				rotateAll(*node, rot, node->trans);
				head_angle = undo_head_s.top();
			}
		}		
		else if(node->m_nodeType == NodeType::JointNode) {
			m = i;
			glm::mat4 rot = undo_s[i].top();
			if(undo_s[i].size() > 1) {
				undo_s[i].pop();
				redo_s[i].push(rot);
			}

			JointNode *jointNode = static_cast<JointNode*>(node);
			if(undo_init_x[i].size() > 1) {
				redo_init_x[i].push(undo_init_x[i].top());
				undo_init_x[i].pop();
				jointNode->m_joint_x.init = undo_init_x[i].top();
				redo_init_y[i].push(undo_init_y[i].top());
				undo_init_y[i].pop();
				jointNode->m_joint_y.init = undo_init_y[i].top();
			}
			
			SceneNode *root = node->children.front();
			rotateAll(*root, inverse(rot), node->trans);
		}
	}
	
	for(int i = 0; i < MAXSTACK; i++) {
		if(undo_s[i].size() > undo_s[m].size())
			undo_s[i].pop();
		if(undo_init_x[i].size() > undo_init_x[m].size())
			undo_init_x[i].pop();
		if(undo_init_y[i].size() > undo_init_y[m].size())
			undo_init_y[i].pop();
	}

	return true;
}

bool A3::Redo(SceneNode &m_rootNode) {
	if(redo_head_s.empty())
		return false;

	int m = 0;
	for(int i = 0; i < m_rootNode.totalSceneNodes(); i++) {
		SceneNode *node = search(m_rootNode, i);
		if(node->m_name == "head") {
			if(redo_head_s.size() > 0) {
				GLfloat current = redo_head_s.top();
				redo_head_s.pop();
				glm::mat4 rot = glm::rotate(mat4(), GLfloat(current - undo_head_s.top()), vec3(0.0f, 1.0f, 0.0f));
				undo_head_s.push(current);
				rotateAll(*node, rot, node->trans);
				head_angle = undo_head_s.top();
			}
		}
		else if(node->m_nodeType == NodeType::JointNode) {
			if(redo_s[i].size() > 0) {
				m = i;
				glm::mat4 rot = redo_s[i].top();
				redo_s[i].pop();
				undo_s[i].push(rot);
				JointNode *jointNode = static_cast<JointNode*>(node);
				undo_init_x[i].push(redo_init_x[i].top());
				redo_init_x[i].pop();
				jointNode->m_joint_x.init = undo_init_x[i].top();
				undo_init_y[i].push(redo_init_y[i].top());
				redo_init_y[i].pop();
				jointNode->m_joint_y.init = undo_init_y[i].top();
				SceneNode *root = node->children.front();
				rotateAll(*root, rot, node->trans);
			}
		}
	}

	for(int i = 0; i < MAXSTACK; i++) {
		if(redo_s[i].size() > redo_s[m].size())
			redo_s[i].pop();
		if(redo_init_x[i].size() > redo_init_x[m].size())
			redo_init_x[i].pop();
		if(redo_init_y[i].size() > redo_init_y[m].size())
			redo_init_y[i].pop();
	}

	return true;
}

void A3::resetJoint(SceneNode &m_rootNode) {
	while(undo_s[0].size() > 1 || undo_head_s.size() > 1) {
		Undo(m_rootNode);
	}

	while(!redo_head_s.empty())
		redo_head_s.pop();

	for(int i = 0; i < MAXSTACK; i++) {
		while(!redo_s[i].empty())
			redo_s[i].pop();

		while(!redo_init_x[i].empty())
			redo_init_x[i].pop();

		while(!redo_init_y[i].empty())
			redo_init_y[i].pop();
	}
}

void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		/*if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}*/
		

		if(ImGui::BeginMenuBar()) {
			if(ImGui::BeginMenu("Application")) {
				if(ImGui::MenuItem("Reset Position")) {
					glm::mat4 translate_mat = glm::translate(mat4(), vec3(origin - m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
					resetPosition(*m_rootNode, translate_mat);
				}

				if(ImGui::MenuItem("Reset Orientation")) {
					glm::vec4 current = m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f);
					glm::mat4 translate_mat = glm::translate(mat4(), vec3(origin - current));
					resetPosition(*m_rootNode, translate_mat);
					
					glm::mat4 rot_mat = glm::inverse(m_rootNode->trans*glm::translate(mat4(), -vec3(origin)));
					resetOrientation(*m_rootNode, rot_mat);
					
					translate_mat = glm::translate(mat4(), vec3(-origin + current));
					resetPosition(*m_rootNode, translate_mat);
				}

				if(ImGui::MenuItem("Reset Joints")) {
					resetJoint(*m_rootNode);
				}

				if(ImGui::MenuItem("Reset All")) {
					glm::mat4 translate_mat = glm::translate(mat4(), vec3(origin - m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
					resetPosition(*m_rootNode, translate_mat);

					glm::vec4 current = m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f);
					translate_mat = glm::translate(mat4(), vec3(origin - current));
					resetPosition(*m_rootNode, translate_mat);
					
					glm::mat4 rot_mat = glm::inverse(m_rootNode->trans*glm::translate(mat4(), -vec3(origin)));
					resetOrientation(*m_rootNode, rot_mat);
					
					translate_mat = glm::translate(mat4(), vec3(-origin + current));
					resetPosition(*m_rootNode, translate_mat);

					resetJoint(*m_rootNode);
					circle = false;
					z_buffer = true;
					backcull = false;
					frontcull = false;
					mode = 1;
					
					for(int i = 0; i < m_rootNode->totalSceneNodes(); i++) {
						SceneNode *node = search(*m_rootNode, i);
						if(node->isSelected == true)
							node->isSelected = false;
					}
				}
				
				if(ImGui::MenuItem("Quit")) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Edit")) {
				if(ImGui::MenuItem("Undo")) {
					allow_undo = Undo(*m_rootNode);
				}

				if(ImGui::MenuItem("Redo")) {
					allow_redo = Redo(*m_rootNode);
				}

				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Options")) {
				if(ImGui::Checkbox("Circle", &circle)) {}
				if(ImGui::Checkbox("Z-buffer", &z_buffer)) {}
				if(ImGui::Checkbox("Backface culling", &backcull)) {}
				if(ImGui::Checkbox("Frontface culling", &frontcull)) {}
				ImGui::EndMenu();
			}
			
			ImGui::EndMenuBar();
		}

		if(ImGui::RadioButton("Positon", &mode, 1)) {}
		
		if(ImGui::RadioButton("Joints", &mode, 2)) {}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	if(!allow_undo) {
		ImGui::OpenPopup("Can't undo");
	}

	if(ImGui::BeginPopupModal("Can't undo")) {
		ImGui::Text("Error: you can't undo!");
		if(ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			allow_undo = true;
		}

		ImGui::EndPopup();
	}

	if(!allow_redo) {
		ImGui::OpenPopup("Can't redo");
	}

	if(ImGui::BeginPopupModal("Can't redo")) {
		ImGui::Text("Error: you can't redo!");
		if(ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			allow_redo = true;
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		if(do_picking) {
			unsigned int idx = node.m_nodeId;
			float r = float(idx&0xff)/255.0f;
			float g = float((idx >> 8)&0xff)/255.0f;
			float b = float((idx >> 16)&0xff)/255.0f;

			location = m_shader.getUniformLocation("material.kd");
			glUniform3f(location, r, g, b);
			CHECK_GL_ERRORS;
		}
		else {
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;


			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			vec3 kd = node.material.kd;
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.ks");
			vec3 ks = node.material.ks;
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, node.material.shininess);
			CHECK_GL_ERRORS;
		}

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	if(z_buffer)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if(backcull && frontcull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
	}
	else if(backcull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else if(frontcull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	else
		glDisable(GL_CULL_FACE);


	renderSceneGraph(*m_rootNode);


	if(circle) {
		glDisable( GL_DEPTH_TEST );
		renderArcCircle();
	}
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	for (SceneNode * node : root.children) {

		if (node->m_nodeType != NodeType::GeometryNode)
			continue;

		GeometryNode * geometryNode = static_cast<GeometryNode *>(node);

		if(geometryNode->isSelected == true) {
			geometryNode->material.kd = vec3(1.0f, 1.0f, 1.0f);
			geometryNode->material.ks = vec3(0.4f, 0.3f, 0.5f);
			geometryNode->material.shininess = 1;
		}
		else
			geometryNode->material = geometryNode->origin;

		updateShaderUniforms(m_shader, *geometryNode, m_view);


		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}

	glBindVertexArray(0);
	CHECK_GL_ERRORS;

	for (SceneNode * node : root.children)
		renderSceneGraph(*node);
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */

/*void translateAll(SceneNode& root, glm::vec3 amount) {
	root.translate(amount);
	for(SceneNode* node: root.children) {
		translateAll(*node, amount);
	}
}*/
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(left || middle || right) {
		if(pre_xpos == 0 && cur_xpos == 0) {
			pre_xpos = xPos;
			pre_ypos = yPos;
		}
		else {
			cur_xpos = xPos;
			cur_ypos = yPos;

			if(mode == 1) {
				if(left) {
					//translateAll(*m_rootNode, vec3((cur_xpos - pre_xpos)/100, -(cur_ypos - pre_ypos)/100, 0.0f));
					m_rootNode->translate(vec3((cur_xpos - pre_xpos)/100, -(cur_ypos - pre_ypos)/100, 0.0f));
				}

				if(middle) {
					//translateAll(*m_rootNode, vec3(0.0f, 0.0f, (cur_ypos - pre_ypos)/100));
					m_rootNode->translate(vec3(0.0f, 0.0f, (cur_ypos - pre_ypos)/100));
				}

				if(right) {
					float fVecX, fVecY, fVecZ;
					float fDiameter = (m_windowWidth < m_windowHeight) ? m_windowWidth*0.5 : m_windowHeight*0.5;
					float iCenterX = m_windowWidth/2, iCenterY = m_windowHeight/2;
					vCalcRotVec(cur_xpos - iCenterX, cur_ypos - iCenterY, pre_xpos - iCenterX, pre_ypos - iCenterY, fDiameter, &fVecX, &fVecY, &fVecZ);
					glm::mat4 rotate;
					vAxisRotMatrix(fVecX, -fVecY, fVecZ, rotate);
					rotateAll(*m_rootNode, glm::transpose(rotate), m_rootNode->trans);
				}
			}
			else if(mode == 2) {
				if(middle) {
					for(int i = 0; i < m_rootNode->totalSceneNodes(); i++) {
						SceneNode *node = search(*m_rootNode, i);
						if((node->m_nodeType == NodeType::JointNode) && (node->isSelected == true)) {
							//GLfloat xangle = (cur_xpos - pre_xpos)/50;
							GLfloat xangle = (cur_ypos - pre_ypos)/50;
							GLfloat zangle = (cur_ypos - pre_ypos)/50;

							glm::mat4 rot_mat1 = glm::rotate(mat4(), xangle, vec3(1.0f, 0.0f, 0.0f));
							glm::mat4 rot_mat2 = glm::rotate(mat4(), zangle, vec3(0.0f, 0.0f, 1.0f));
							SceneNode *root = node->children.front();

							JointNode *jointNode = static_cast<JointNode*>(node);
							GLfloat xRange = xangle + jointNode->m_joint_x.init;
							GLfloat zRange = zangle + jointNode->m_joint_y.init;
							if((xRange > jointNode->m_joint_x.min) && (xRange < jointNode->m_joint_x.max)) {
								jointNode->m_joint_x.init = xRange;
								rotateAll(*root, rot_mat1, node->trans);
								m_joint[i] = rot_mat1*m_joint[i];
							}
							/*
							if((zRange > jointNode->m_joint_y.min) && (zRange < jointNode->m_joint_y.max)) {
								jointNode->m_joint_y.init = zRange;
								rotateAll(*root, rot_mat2, node->trans);
								m_joint[i] = rot_mat2*m_joint[i];
							}*/
						}
					}
				}

				if(right) {
					for(int i = 0; i < m_rootNode->totalSceneNodes(); i++) {
						SceneNode *node = search(*m_rootNode, i);
						if((node->m_name == "head") && (node->isSelected == true)) {
							GLfloat xangle = (cur_xpos - pre_xpos)/50;
							glm::mat4 rot_mat = glm::rotate(mat4(), xangle, vec3(0.0f, 1.0f, 0.0f));
							if((xangle + head_angle > -3.14/4) && (xangle + head_angle < 3.14/4)) {
								head_angle += xangle;
								rotateAll(*node, rot_mat, node->trans);
							}
						}
					}
				}
			}

			pre_xpos = cur_xpos;
			pre_ypos = cur_ypos;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(!ImGui::IsMouseHoveringAnyWindow()) {
		if(button == GLFW_MOUSE_BUTTON_LEFT) {
			if(actions == GLFW_PRESS) {
				left = true;
				pre_xpos = 0.0f;
				cur_xpos = 0.0f;

				if(mode == 2) {
					double xpos, ypos;
					glfwGetCursorPos(m_window, &xpos, &ypos);

					do_picking = true;

					uploadCommonSceneUniforms();
					glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glClearColor(0.35f, 0.35f, 0.35f, 1.0f);

					draw();
					CHECK_GL_ERRORS;

					xpos *= double(m_framebufferWidth)/double(m_windowWidth);
					ypos = m_windowHeight - ypos;
					ypos *= double(m_framebufferHeight)/double(m_windowHeight);
					GLubyte buffer[4] = {0, 0, 0, 0};
					glReadBuffer(GL_BACK);
					glReadPixels(int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
					CHECK_GL_ERRORS;

					unsigned int idx = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);
					if(idx < m_rootNode->totalSceneNodes()) {
						SceneNode *node = search(*m_rootNode, idx);
						if(node->parent->m_nodeType == NodeType::JointNode) {
							node->isSelected = !node->isSelected;
							node->parent->isSelected = !node->parent->isSelected;
						}
					}

					do_picking = false;
					CHECK_GL_ERRORS;
				}
			}
			else if(actions == GLFW_RELEASE)
				left = false;
		}
		
		if(button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if(actions == GLFW_PRESS) {
				middle = true;
				pre_xpos = 0.0f;
				cur_xpos = 0.0f;

				if(mode == 2) {
					for(int i = 0; i < MAXSTACK; i++)
						m_joint[i] = mat4();

					while(!redo_head_s.empty())
						redo_head_s.pop();

					for(int i = 0; i < MAXSTACK; i++) {
						while(!redo_s[i].empty())
							redo_s[i].pop();

						while(!redo_init_x[i].empty())
							redo_init_x[i].pop();

						while(!redo_init_y[i].empty())
							redo_init_y[i].pop();
					}
				}

				
			}
			else if(actions == GLFW_RELEASE) {
				middle = false;

				if(mode == 2) {
					undo_head_s.push(undo_head_s.top());
					for(int i = 0; i < MAXSTACK; i++)
					{
						undo_s[i].push(m_joint[i]);

						SceneNode *node = search(*m_rootNode, i);
						if(node == NULL || node->m_nodeType != NodeType::JointNode) {
							undo_init_x[i].push(0);
							undo_init_y[i].push(0);
						}
						else {
							JointNode *jointNode = static_cast<JointNode*>(node);
							undo_init_x[i].push(jointNode->m_joint_x.init);
							undo_init_y[i].push(jointNode->m_joint_y.init);
						}
					}
				}
			}
		}
		
		if(button == GLFW_MOUSE_BUTTON_RIGHT) {
			if(actions == GLFW_PRESS) {
				right = true;
				pre_xpos = 0.0f;
				cur_xpos = 0.0f;

				if(mode == 2) {
					while(!redo_head_s.empty())
						redo_head_s.pop();

					for(int i = 0; i < MAXSTACK; i++) {
						while(!redo_s[i].empty())
							redo_s[i].pop();

						while(!redo_init_x[i].empty())
							redo_init_x[i].pop();

						while(!redo_init_y[i].empty())
							redo_init_y[i].pop();
					}
				}
			}
			else if(actions == GLFW_RELEASE) {
				right = false;

				if(mode == 2) {
					undo_head_s.push(head_angle);
					for(int i = 0; i < MAXSTACK; i++)
					{
						undo_s[i].push(mat4());

						SceneNode *node = search(*m_rootNode, i);
						if(node == NULL || node->m_nodeType != NodeType::JointNode) {
							undo_init_x[i].push(0);
							undo_init_y[i].push(0);
						}
						else {
							JointNode *jointNode = static_cast<JointNode*>(node);
							undo_init_x[i].push(jointNode->m_joint_x.init);
							undo_init_y[i].push(jointNode->m_joint_y.init);
						}
					}
				}
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}

		if(key ==  GLFW_KEY_I) {
			glm::mat4 translate_mat = glm::translate(mat4(), vec3(origin - m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
			resetPosition(*m_rootNode, translate_mat);
		}

		if(key == GLFW_KEY_O) {
			glm::vec4 current = m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::mat4 translate_mat = glm::translate(mat4(), vec3(origin - current));
			resetPosition(*m_rootNode, translate_mat);
					
			glm::mat4 rot_mat = glm::inverse(m_rootNode->trans*glm::translate(mat4(), -vec3(origin)));
			resetOrientation(*m_rootNode, rot_mat);
					
			translate_mat = glm::translate(mat4(), vec3(-origin + current));
			resetPosition(*m_rootNode, translate_mat);
		}

		if(key == GLFW_KEY_N) {
			resetJoint(*m_rootNode);
		}

		if(key == GLFW_KEY_A) {
			glm::mat4 translate_mat = glm::translate(mat4(), vec3(origin - m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
			resetPosition(*m_rootNode, translate_mat);

			glm::vec4 current = m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f);
			translate_mat = glm::translate(mat4(), vec3(origin - current));
			resetPosition(*m_rootNode, translate_mat);
					
			glm::mat4 rot_mat = glm::inverse(m_rootNode->trans*glm::translate(mat4(), -vec3(origin)));
			resetOrientation(*m_rootNode, rot_mat);
					
			translate_mat = glm::translate(mat4(), vec3(-origin + current));
			resetPosition(*m_rootNode, translate_mat);

			resetJoint(*m_rootNode);
			circle = false;
			z_buffer = true;
			backcull = false;
			frontcull = false;
			mode = 1;
			
			for(int i = 0; i < m_rootNode->totalSceneNodes(); i++) {
				SceneNode *node = search(*m_rootNode, i);
				if(node->isSelected == true)
					node->isSelected = false;
			}
		}

		if(key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if(key == GLFW_KEY_U) {
			allow_undo = Undo(*m_rootNode);
		}

		if(key == GLFW_KEY_R) {
			allow_redo = Redo(*m_rootNode);
		}

		if(key == GLFW_KEY_C) {
			circle = !circle;
		}

		if(key == GLFW_KEY_Z) {
			z_buffer = !z_buffer;
		}

		if(key == GLFW_KEY_B) {
			backcull = !backcull;
		}

		if(key == GLFW_KEY_F) {
			frontcull = !frontcull;
		}

		if(key == GLFW_KEY_P) {
			mode = 1;
		}

		if(key == GLFW_KEY_J) {
			mode = 2;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
