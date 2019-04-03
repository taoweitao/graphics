#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

void printMat4(glm::mat4 M) {
	for(int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			std::cout << M[i][j] << " ";
		std::cout << std::endl;
	}
}

glm::mat4 myLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up) {
	vec3 f = glm::normalize(center - eye);
	vec3 UP = glm::normalize(up);
	vec3 s = glm::normalize(glm::cross(f, UP));
	vec3 u = glm::normalize(glm::cross(s, f));

	glm::mat4 view;
	view = glm::transpose(glm::mat4(glm::vec4(s, -glm::dot(s, eye)), glm::vec4(u, -glm::dot(u, eye)), glm::vec4(-f, glm::dot(f,eye)), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	
	return view;
}

glm::mat4 myPerspective(GLfloat fov, GLfloat aspect, GLfloat n, GLfloat f) {
	glm::mat4 proj;
	proj = glm::transpose(glm::mat4(glm::vec4(1./glm::tan(fov/2)/aspect, 0, 0, 0), glm::vec4(0, 1./glm::tan(fov/2), 0, 0), glm::vec4(0, 0, -(f + n)/(f - n), -2*f*n/(f - n)), glm::vec4(0, 0, -1, 0)));
	
	return proj;
}

glm::mat4 myTranslate(glm::mat4 matrix, glm::vec3 vector) {
	glm::mat4 translate;
	translate = glm::transpose(glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, vector.x), glm::vec4(0.0f, 1.0f, 0.0f, vector.y), glm::vec4(0.0f, 0.0f, 1.0f, vector.z), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));

	return translate*matrix;
}

glm::mat4 myRotate(glm::mat4 matrix, GLfloat angles, glm::vec3 axis) {
	axis = glm::normalize(axis);
	GLfloat x = axis.x;
	GLfloat y = axis.y;
	GLfloat z = axis.z;
	GLfloat A = angles;
	
	glm::mat4 rotate;
	rotate= glm::transpose(glm::mat4(glm::vec4(cos(A) + x*x*(1 - cos(A)), x*y*(1 - cos(A)) - z*sin(A), x*z*(1 - cos(A)) + y*sin(A), 0.0f), glm::vec4(y*x*(1 - cos(A)) + z*sin(A), cos(A) + y*y*(1 - cos(A)), y*z*(1 - cos(A)) - x*sin(A), 0.0f), glm::vec4(z*x*(1 - cos(A)) - y*sin(A), z*y*(1 - cos(A)) + x*sin(A), cos(A) + z*z*(1 - cos(A)), 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));

	return rotate*matrix;
}

glm::mat4 myScale(glm::mat4 matrix, glm::vec3 vector) {
	glm::mat4 scale;
	scale = glm::transpose(glm::mat4(glm::vec4(vector.x, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, vector.y, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, vector.z, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));

	return scale*matrix;	
}

glm::vec2 A2::mapViewport(const glm::vec2 point) {
	GLfloat minX = std::min(viewport[0].x, viewport[1].x);
	GLfloat maxX = std::max(viewport[0].x, viewport[1].x);
	GLfloat minY = std::min(viewport[0].y, viewport[1].y);
	GLfloat maxY = std::max(viewport[0].y, viewport[1].y);
	glm::vec2 v;
	v.x = (maxX + minX)/2 + (maxX - minX)*point.x/2;
	v.y = (maxY + minY)/2 + (maxY - minY)*point.y/2;

	return v;
}

bool A2::lineClipping(glm::vec2& point1, glm::vec2& point2) {
	GLfloat minX = std::min(viewport[0].x, viewport[1].x);
	GLfloat maxX = std::max(viewport[0].x, viewport[1].x);
	GLfloat minY = std::min(viewport[0].y, viewport[1].y);
	GLfloat maxY = std::max(viewport[0].y, viewport[1].y);

	if(point1.x < minX && point2.x < minX || point1.y < minY && point2.y < minY || point1.x > maxX && point2.x > maxX || point1.y > maxY && point2.y > maxY)
		return false;
	
	GLfloat tmin = 0, tmax = 1;
	if(point1.x <= point2.x && point1.y <= point2.y) {
		if(point1.x <= minX && point2.x > minX)
			tmin = std::max(tmin, (point1.x - minX)/(point1.x - point2.x));
		if(point1.x < maxX && point2.x >= maxX)
			tmax = std::min(tmax, (point1.x - maxX)/(point1.x - point2.x));
		if(point1.y <= minY && point2.y > minY)
			tmin = std::max(tmin, (point1.y - minY)/(point1.y - point2.y));
		if(point1.y < maxY && point2.y >= maxY)
			tmax = std::min(tmax, (point1.y - maxY)/(point1.y - point2.y));
	}
	else if(point1.x <= point2.x && point1.y > point2.y) {
		if(point1.x <= minX && point2.x > minX)
			tmin = std::max(tmin, (point1.x - minX)/(point1.x - point2.x));
		if(point1.x < maxX && point2.x >= maxX)
			tmax = std::min(tmax, (point1.x - maxX)/(point1.x - point2.x));
		if(point2.y <= minY && point1.y > minY)
			tmax = std::min(tmax, (point1.y - minY)/(point1.y - point2.y));
		if(point2.y < maxY && point1.y >= maxY)
			tmin = std::max(tmin, (point1.y - maxY)/(point1.y - point2.y));
	}
	else
		return lineClipping(point2, point1);

	if(tmin >= tmax)
		return false;

	glm::vec2 new_point1 = point1 + tmin*(point2 - point1);
	glm::vec2 new_point2 = point1 + tmax*(point2 - point1);
	point1 = new_point1;
	point2 = new_point2;

	return true;
}

bool A2::volumeClipping(glm::vec3& point1, glm::vec3& point2) {
	if(point1.z > -near && point2.z > -near || point1.z < -far && point2.z < -far)
		return false;
	
	GLfloat tmin = 0, tmax = 1;
	if(point1.z >= point2.z) {
		if(point1.z >= -near && point2.z < -near)
			tmin = std::max(tmin, (point1.z + near)/(point1.z - point2.z));
		if(point1.z >= -far && point2.z < -far)
			tmax = std::min(tmax, (point1.z + far)/(point1.z - point2.z));
	}
	else
		return volumeClipping(point2, point1);

	if(tmin >= tmax)
		return false;
	
	glm::vec3 new_point1 = point1 + tmin*(point2 - point1);
	glm::vec3 new_point2 = point1 + tmax*(point2 - point1);
	point1 = new_point1;
	point2 = new_point2;

	return true;
}

void A2::initValue() {
	cubic_center = glm::vec3(0.0f);
	world_center = glm::vec3(0.0f);

	for(int i = 0; i < 8; i++) {
		int a = (i & 0x4) >> 2;
		int b = (i & 0x2) >> 1;
		int c = (i & 0x1);

		if(a == 0)
			cubic[i].x = -1.0f;
		else
			cubic[i].x = 1.0f;
		
		if(b == 0)
			cubic[i].y = -1.0f;
		else
			cubic[i].y = 1.0f;

		if(c == 0)
			cubic[i].z = -1.0f;
		else
			cubic[i].z = 1.0f;
	}

	eyes = glm::vec3(4.0f, 4.0f, 4.0f);
	near = 1.0f;
	far = 10.0f;
	fov = 30.0f;
	aspect = 1.0;

	model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	view = myLookAt(eyes, world_center, glm::vec3(0.0f, 1.0f, 0.0f));	
	proj = myPerspective(glm::radians(fov), aspect, near, far);

	world_coor[0] = glm::vec3(1.0f, 0.0f, 0.0f);
	world_coor[1] = glm::vec3(0.0f, 1.0f, 0.0f);
	world_coor[2] = glm::vec3(0.0f, 0.0f, 1.0f);

	model_coor[0] = glm::normalize(cubic[5] + cubic[6] - cubic_center);
	model_coor[1] = glm::normalize(cubic[2] + cubic[7] - cubic_center);
	model_coor[2] = glm::normalize(cubic[1] + cubic[7] - cubic_center);	
	
	mode = 3;
	left = false;
	middle = false;
	right = false;
	xbound = 1.0f;
	ybound = 1.0f;
	zbound = 1.0f;

	viewport[0] = glm::vec2(0.9, 0.9);
	viewport[1] = glm::vec2(-0.9, -0.9);
}

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{
	initValue();
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();
	
	// draw the view port
	setLineColour(glm::vec3(1.0f, 1.0f, 0.0f));
	drawLine(viewport[0], vec2(viewport[0].x, viewport[1].y));
	drawLine(viewport[0], vec2(viewport[1].x, viewport[0].y));
	drawLine(viewport[1], vec2(viewport[0].x, viewport[1].y));
	drawLine(viewport[1], vec2(viewport[1].x, viewport[0].y));
	
	// draw the cube
	glm::vec4 cubic_proj[8];
	for(int i = 0; i < 8; i++)
		cubic_proj[i] = view*model*vec4(cubic[i], 1.0f);

	glm::vec3 nnp1[12], nnp2[12];
	nnp1[0] = vec3(cubic_proj[0]), nnp2[0] = vec3(cubic_proj[1]);
	nnp1[1] = vec3(cubic_proj[0]), nnp2[1] = vec3(cubic_proj[2]);
	nnp1[2] = vec3(cubic_proj[0]), nnp2[2] = vec3(cubic_proj[4]);
	nnp1[3] = vec3(cubic_proj[1]), nnp2[3] = vec3(cubic_proj[3]);
	nnp1[4] = vec3(cubic_proj[1]), nnp2[4] = vec3(cubic_proj[5]);
	nnp1[5] = vec3(cubic_proj[2]), nnp2[5] = vec3(cubic_proj[3]);
	nnp1[6] = vec3(cubic_proj[2]), nnp2[6] = vec3(cubic_proj[6]);
	nnp1[7] = vec3(cubic_proj[3]), nnp2[7] = vec3(cubic_proj[7]);
	nnp1[8] = vec3(cubic_proj[4]), nnp2[8] = vec3(cubic_proj[5]);
	nnp1[9] = vec3(cubic_proj[4]), nnp2[9] = vec3(cubic_proj[6]);
	nnp1[10] = vec3(cubic_proj[5]), nnp2[10] = vec3(cubic_proj[7]);
	nnp1[11] = vec3(cubic_proj[6]), nnp2[11] = vec3(cubic_proj[7]);

	bool nflag[12];
	for(int i = 0; i < 12; i++)
		nflag[i] = volumeClipping(nnp1[i], nnp2[i]);

	glm::vec2 np1[12], np2[12];
	setLineColour(glm::vec3(1.0f, 1.0f, 1.0f));
	for(int i = 0; i < 12; i++) {
		glm::vec4 nnnp1 = proj*vec4(nnp1[i], 1.0f);
		nnnp1 = nnnp1 / nnnp1[3];
		np1[i] = mapViewport(glm::vec2(nnnp1));
		
		glm::vec4 nnnp2 = proj*vec4(nnp2[i], 1.0f);
		nnnp2 = nnnp2 / nnnp2[3];
		np2[i] = mapViewport(glm::vec2(nnnp2));

		if(nflag[i] && lineClipping(np1[i], np2[i]))
			drawLine(np1[i], np2[i]);
	}

	// draw the world coordinate
	glm::vec4 world_proj[3];
	glm::vec4 world_center_proj = view*vec4(world_center, 1.0f);
	glm::vec3 wwp1[3], wwp2[3];
	bool wflag[3];
	for(int i = 0; i < 3; i++) {
		world_proj[i] = view*vec4(world_coor[i], 1.0f);
		wwp1[i] = vec3(world_center_proj), wwp2[i] = vec3(world_proj[i]);
		wflag[i] = volumeClipping(wwp1[i], wwp2[i]);
	}

	glm::vec2 wp1[3], wp2[3];
	for(int i = 0; i < 3; i++) {
		setLineColour(glm::vec3((i + 2) >> 2, ((i + 2) >> 1)&0x1, (i + 2)&0x1));	
		glm::vec4 wwwp1 = proj*vec4(wwp1[i], 1.0f);
		wwwp1 = wwwp1 / wwwp1[3];
		wp1[i] = mapViewport(glm::vec2(wwwp1));

		glm::vec4 wwwp2 = proj*vec4(wwp2[i], 1.0f);
		wwwp2 = wwwp2 / wwwp2[3];
		wp2[i] = mapViewport(glm::vec2(wwwp2));
		if(wflag[i] && lineClipping(wp1[i], wp2[i]))
			drawLine(wp1[i], wp2[i]);
	}

	// draw the model coordinate
	glm::vec4 model_proj[3];
	glm::vec4 model_center_proj = view*model*vec4(cubic_center, 1.0f);
	glm::vec3 mmp1[3], mmp2[3];
	bool mflag[3];
	for(int i = 0; i < 3; i++) {
		//model_proj[i] = view*model*vec4(model_coor[i], 1.0f);
		model_proj[i] = model_center_proj + view*glm::normalize(model*vec4(model_coor[i] - cubic_center, 0.0f));
		mmp1[i] = vec3(model_center_proj), mmp2[i] = vec3(model_proj[i]);
		mflag[i] = volumeClipping(mmp1[i], mmp2[i]);
	}

	glm::vec2 mp1[3], mp2[3];
	for(int i = 0; i < 3; i++) {
		setLineColour(glm::vec3((i + 2) >> 2, ((i + 2) >> 1)&0x1, (i + 2)&0x1));	
		glm::vec4 mmmp1 = proj*vec4(mmp1[i], 1.0f);
		mmmp1 = mmmp1 / mmmp1[3];
		mp1[i] = mapViewport(glm::vec2(mmmp1));

		glm::vec4 mmmp2 = proj*vec4(mmp2[i], 1.0f);
		mmmp2 = mmmp2 / mmmp2[3];
		mp2[i] = mapViewport(glm::vec2(mmmp2));
		if(mflag[i] && lineClipping(mp1[i], mp2[i]))
			drawLine(mp1[i], mp2[i]);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if(ImGui::Button("Reset")) {
			initValue();
		}

		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		
		if(ImGui::RadioButton("Rotate View", &mode, 0)) {
			//std::cout << mode << std::endl;
		}

		if(ImGui::RadioButton("Translate View", &mode, 1)) {
			//std::cout << mode << std::endl;
		}

		if(ImGui::RadioButton("Perspective", &mode, 2)) {
			//std::cout << mode << std::endl;
		}

		if(ImGui::RadioButton("Rotate Model", &mode, 3)) {
			//std::cout << mode << std::endl;
		}

		if(ImGui::RadioButton("Translate Model", &mode, 4)) {
			//std::cout << mode << std::endl;
		}

		if(ImGui::RadioButton("Scale Model", &mode, 5)) {
			//std::cout << mode << std::endl;
		}

		if(ImGui::RadioButton("Viewport", &mode, 6)) {
			//std::cout << mode << std::endl;
		}

		ImGui::Text("Near Plane: %f", near);
		ImGui::Text("Far Plane: %f", far);
		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(left || middle || right) {
		if(pre_xpos == 0 && cur_xpos == 0) {
			pre_xpos = xPos;
			if(mode == 6) {
				GLfloat x = xPos/m_windowWidth*2 - 1;
				GLfloat y = (1 - yPos/m_windowHeight)*2 - 1;
				if(x >= -1 && x <= 1 && y >= -1 && y <= 1)
					viewport[0] = glm::vec2(x, y);
			}
		}
		else {
			cur_xpos = xPos;
			if(mode == 0) {
				if(left)
					view = myRotate(view, (cur_xpos - pre_xpos)/50.0, glm::vec3(1.0f, 0.0f, 0.0f));
				if(middle)
					view = myRotate(view, (cur_xpos - pre_xpos)/50.0, glm::vec3(0.0f, 1.0f, 0.0f));
				if(right)
					view = myRotate(view, (cur_xpos - pre_xpos)/50.0, glm::vec3(0.0f, 0.0f, 1.0f));
			}
			else if(mode == 1) {
				if(left)
					view = myTranslate(view, glm::vec3((cur_xpos - pre_xpos)/50.0, 0.0f, 0.0f));
				if(middle)
					view = myTranslate(view, glm::vec3(0.0f, (cur_xpos - pre_xpos)/50.0, 0.0f));
				if(right)
					view = myTranslate(view, glm::vec3(0.0f, 0.0f, (cur_xpos - pre_xpos)/50.0));
			}
			else if(mode == 2) {
				if(left) {
					if(fov + (cur_xpos - pre_xpos)/10.0 >= 5 && fov + (cur_xpos - pre_xpos)/10.0 <= 160) {
						fov += ((cur_xpos - pre_xpos)/10.0);
						proj = myPerspective(glm::radians(fov), aspect, near, far);
					}
				}
				if(middle) {
					//if(near + (cur_xpos - pre_xpos)/10.0 >= 0) {
						near += ((cur_xpos - pre_xpos)/10.0);
						proj = myPerspective(glm::radians(fov), aspect, near, far);
					//}
				}
				if(right) {
					//if(far + (cur_xpos - pre_xpos)/10.0 >= 0) {
						far += ((cur_xpos - pre_xpos)/10.0);
						proj = myPerspective(glm::radians(fov), aspect, near, far);
					//}
				}
			}
			else if(mode == 3) {
				glm::vec4 storeC = model*glm::vec4(cubic_center, 1.0f);
				if(left) {
					glm::vec4 storeX = model*glm::vec4(model_coor[0], 1.0f);
					model = myTranslate(model, -glm::vec3(storeC));
					model = myRotate(model, (cur_xpos - pre_xpos)/50.0, glm::vec3(storeX - storeC));
					model = myTranslate(model, glm::vec3(storeC));
				}
				if(middle) {
					glm::vec4 storeY = model*glm::vec4(model_coor[1], 1.0f);
					model = myTranslate(model, -glm::vec3(storeC));
					model = myRotate(model, (cur_xpos - pre_xpos)/50.0, glm::vec3(storeY - storeC));
					model = myTranslate(model, glm::vec3(storeC));
				}
				if(right) {
					glm::vec4 storeZ = model*glm::vec4(model_coor[2], 1.0f);
					model = myTranslate(model, -glm::vec3(storeC));
					model = myRotate(model, (cur_xpos - pre_xpos)/50.0, glm::vec3(storeZ - storeC));
					model = myTranslate(model, glm::vec3(storeC));
				}
			}
			else if(mode == 4) {
				glm::vec4 storeC = model*glm::vec4(cubic_center, 1.0f);
				if(left) {
					model = myTranslate(model, -glm::vec3(storeC));
					glm::mat4 storeR = model;
					model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					model = myTranslate(model, glm::vec3((cur_xpos - pre_xpos)/100.0, 0.0f, 0.0f));
					model = storeR*model;
					model = myTranslate(model, glm::vec3(storeC));
				}
				if(middle) {
					model = myTranslate(model, -glm::vec3(storeC));
					glm::mat4 storeR = model;
					model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					model = myTranslate(model, glm::vec3(0.0f, (cur_xpos - pre_xpos)/100.0, 0.0f));
					model = storeR*model;
					model = myTranslate(model, glm::vec3(storeC));
				}
				if(right) {	
					model = myTranslate(model, -glm::vec3(storeC));
					glm::mat4 storeR = model;
					model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					model = myTranslate(model, glm::vec3(0.0f, 0.0f, (cur_xpos - pre_xpos)/100.0));
					model = storeR*model;
					model = myTranslate(model, glm::vec3(storeC));
				}
			}
			else if(mode == 5) {
				glm::vec4 storeC = model*glm::vec4(cubic_center, 1.0f);
				if(left) {
					if(xbound*(1.0f + (cur_xpos - pre_xpos)/100.0) <= 2 && xbound*(1.0f + (cur_xpos - pre_xpos)/100.0) >= 0.2) {
						model = myTranslate(model, -glm::vec3(storeC));
						glm::mat4 storeR = model;
						model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
						model = myScale(model, glm::vec3(1.0f + (cur_xpos - pre_xpos)/100.0, 1.0f, 1.0f));
						model = storeR*model;
						model = myTranslate(model, glm::vec3(storeC));
						xbound *= (1.0f + (cur_xpos - pre_xpos)/100.0);
					}
				}
				if(middle) {
					if(ybound*(1.0f + (cur_xpos - pre_xpos)/100.0) <= 2 && ybound*(1.0f + (cur_xpos - pre_xpos)/100.0) >= 0.2) {
						model = myTranslate(model, -glm::vec3(storeC));
						glm::mat4 storeR = model;
						model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
						model = myScale(model, glm::vec3(1.0f, 1.0f + (cur_xpos - pre_xpos)/100.0, 1.0f));
						model = storeR*model;
						model = myTranslate(model, glm::vec3(storeC));
						ybound *= (1.0f + (cur_xpos - pre_xpos)/100.0);
					}
				}
				if(right) {
					if(zbound*(1.0f + (cur_xpos - pre_xpos)/100.0) <= 2 && zbound*(1.0f + (cur_xpos - pre_xpos)/100.0) >= 0.2) {
						model = myTranslate(model, -glm::vec3(storeC));
						glm::mat4 storeR = model;
						model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
						model = myScale(model, glm::vec3(1.0f, 1.0f, 1.0f + (cur_xpos - pre_xpos)/100.0));
						model = storeR*model;
						model = myTranslate(model, glm::vec3(storeC));
						zbound *= (1.0f + (cur_xpos - pre_xpos)/100.0);
					}
				}
			}
			else if(mode == 6) {
				GLfloat x = xPos/m_windowWidth*2 - 1;
				GLfloat y = (1 - yPos/m_windowHeight)*2 - 1;
				if(x >= -1 && x <= 1 && y >= -1 && y <= 1)
					viewport[1] = glm::vec2(x, y);
			}

			pre_xpos = cur_xpos;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
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
			}
			else if(actions == GLFW_RELEASE) {
				left = false;
			}
		}
		
		if(button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if(actions == GLFW_PRESS) {
				middle = true;
				pre_xpos = 0.0f;
				cur_xpos = 0.0f;
			}
			else if(actions == GLFW_RELEASE) {
				middle = false;
			}
		}
		
		if(button == GLFW_MOUSE_BUTTON_RIGHT) {
			if(actions == GLFW_PRESS) {
				right = true;
				pre_xpos = 0.0f;
				cur_xpos = 0.0f;
			}
			else if(actions == GLFW_RELEASE) {
				right = false;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
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
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(action == GLFW_PRESS) {
		if(key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if(key == GLFW_KEY_A) {
			initValue();
		}

		if(key == GLFW_KEY_O) {
			mode = 0;
		}

		if(key == GLFW_KEY_N) {
			mode = 1;
		}

		if(key == GLFW_KEY_P) {
			mode = 2;
		}

		if(key == GLFW_KEY_R) {
			mode = 3;
		}

		if(key == GLFW_KEY_T) {
			mode = 4;
		}

		if(key == GLFW_KEY_S) {
			mode = 5;
		}

		if(key == GLFW_KEY_V) {
			mode = 6;
		}
	}

	return eventHandled;
}
