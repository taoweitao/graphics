#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <time.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
{
	indicator_color.r = 0.0f;
	indicator_color.g = 0.0f;
	indicator_color.b = 0.0f;

	for(int i = 0; i < 8; i++) {
		colour[i].r = (i&0x4) >> 2, colour[i].g = (i&0x2) >> 1, colour[i].b = i&0x1;
	}

	m_active_x = 0;
	m_active_z = 0;
	
	for(int i = 0; i < DIM + 2; i++) {
		for (int j = 0; j < DIM + 2; j++) {
			num[i][j] = 0;
			color_type[i][j] = 0;
			m_color[i][j].r = 0.0f;
			m_color[i][j].g = 0.0f;
			m_color[i][j].b = 0.0f;
		}
	}

	srand((unsigned)time(NULL));
	is_copy = false;
	num_copy = 1;
	indicator = 0;
	bound = 1;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();

	// Set up initial view and projection and model matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );

	model = glm::translate( model, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	glEnable(GL_DEPTH_TEST);
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::updateCube(float x, float y, float z)
{
	size_t sz = 3*3*12;
	float *verts = new float[sz];

	verts[0] = x, verts[1] = y, verts[2] = z;
	verts[3] = x + 1, verts[4] = y, verts[5] = z;
	verts[6] = x, verts[7] = y + 1, verts[8] = z;
	verts[9] = x + 1, verts[10] = y, verts[11] = z;
	verts[12] = x, verts[13] = y + 1, verts[14] = z;
	verts[15] = x + 1, verts[16] = y + 1, verts[17] = z;

	verts[18] = x, verts[19] = y, verts[20] = z + 1;
	verts[21] = x + 1, verts[22] = y, verts[23] = z + 1;
	verts[24] = x, verts[25] = y + 1, verts[26] = z + 1;
	verts[27] = x + 1, verts[28] = y, verts[29] = z + 1;
	verts[30] = x, verts[31] = y + 1, verts[32] = z + 1;
	verts[33] = x + 1, verts[34] = y + 1, verts[35] = z + 1;

	verts[36] = x, verts[37] = y, verts[38] = z;
	verts[39] = x, verts[40] = y + 1, verts[41] = z;
	verts[42] = x, verts[43] = y, verts[44] = z + 1;
	verts[45] = x, verts[46] = y + 1, verts[47] = z;
	verts[48] = x, verts[49] = y, verts[50] = z + 1;
	verts[51] = x, verts[52] = y + 1, verts[53] = z + 1;

	verts[54] = x + 1, verts[55] = y, verts[56] = z;
	verts[57] = x + 1, verts[58] = y + 1, verts[59] = z;
	verts[60] = x + 1, verts[61] = y, verts[62] = z + 1;
	verts[63] = x + 1, verts[64] = y + 1, verts[65] = z;
	verts[66] = x + 1, verts[67] = y, verts[68] = z + 1;
	verts[69] = x + 1, verts[70] = y + 1, verts[71] = z + 1;

	verts[72] = x, verts[73] = y, verts[74] = z;
	verts[75] = x + 1, verts[76] = y, verts[77] = z;
	verts[78] = x, verts[79] = y, verts[80] = z + 1;
	verts[81] = x + 1, verts[82] = y, verts[83] = z;
	verts[84] = x, verts[85] = y, verts[86] = z + 1;
	verts[87] = x + 1, verts[88] = y, verts[89] = z + 1;
	
	verts[90] = x, verts[91] = y + 1, verts[92] = z;
	verts[93] = x + 1, verts[94] = y + 1, verts[95] = z;
	verts[96] = x, verts[97] = y + 1, verts[98] = z + 1;
	verts[99] = x + 1, verts[100] = y + 1, verts[101] = z;
	verts[102] = x, verts[103] = y + 1, verts[104] = z + 1;
	verts[105] = x + 1, verts[106] = y + 1, verts[107] = z + 1;
	
	glGenVertexArrays(1, &m_cube_vao);
	glBindVertexArray(m_cube_vao);

	glGenBuffers(1, &m_cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz*sizeof(float), verts, GL_STATIC_DRAW);

	GLint posAttrib = m_shader.getAttribLocation("position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete [] verts;
}

void A1::reset() {
	current_col = 0;

	indicator_color.r = 0.0f;
	indicator_color.g = 0.0f;
	indicator_color.b = 0.0f;
	
	for(int i = 0; i < 8; i++) {
		colour[i].r = (i&0x4) >> 2, colour[i].g = (i&0x2) >> 1, colour[i].b = i&0x1;
	}

	m_active_x = 0;
	m_active_z = 0;
	
	for(int i = 0; i < DIM + 2; i++) {
		for (int j = 0; j < DIM + 2; j++) {
			num[i][j] = 0;
			color_type[i][j] = 0;
			m_color[i][j].r = 0.0f;
			m_color[i][j].g = 0.0f;
			m_color[i][j].b = 0.0f;
		}
	}

	srand((unsigned)time(NULL));
	is_copy = false;
	num_copy = 1;
	indicator = 0;
	bound = 1;

	model = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-(float)DIM/2.0f, 0.0f, -(float)DIM/2.0f));
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if(ImGui::Button("Reset")) {
			reset();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		for(int i = 0; i < 8; i++) {
			ImGui::ColorEdit3("##Colour", glm::value_ptr(colour[i]));
			ImGui::SameLine();
			ImGui::PushID(i);
			if(ImGui::RadioButton("##Col", &current_col, i)) {
				GLuint x = m_active_x;
				GLuint z = m_active_z;
				color_type[x][z] = current_col;
				m_color[x][z] = colour[i];
				for(int j = 0; j < DIM + 2; j++) {
					for (int k = 0; k < DIM + 2; k++) {
						if(num[j][k] == 0) {
							color_type[j][k] = current_col;
							m_color[j][k] = colour[i];
						}
					}
				}
			}
			ImGui::PopID();
		}

		if(ImGui::SliderFloat("Red Channel", &colour[current_col].r, 0.0f, 1.0f)) {
			for(int j = 0; j < DIM + 2; j++) {
				for (int k = 0; k < DIM + 2; k++) {
					if(color_type[j][k] == current_col)
						m_color[j][k].r = colour[current_col].r;
				}
			}
		}

		if(ImGui::SliderFloat("Green Channel", &colour[current_col].g, 0.0f, 1.0f)) {
			for(int j = 0; j < DIM + 2; j++) {
				for (int k = 0; k < DIM + 2; k++) {
					if(color_type[j][k] == current_col)
						m_color[j][k].g = colour[current_col].g;
				}
			}
		}
		
		if(ImGui::SliderFloat("Blue Channel", &colour[current_col].b, 0.0f, 1.0f)) {
			for(int j = 0; j < DIM + 2; j++) {
				for (int k = 0; k < DIM + 2; k++) {
					if(color_type[j][k] == current_col)
						m_color[j][k].b = colour[current_col].b;
				}
			}
		}

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( model ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		for(int i = 0; i < DIM; i++) {
			for(int j = 0; j < DIM; j++) {
				int k;
				for(k = 0; k < num[i][j]; k++) {
					updateCube(GLfloat(i), GLfloat(k), GLfloat(j));
					glBindVertexArray(m_cube_vao);
					glUniform3f(col_uni, m_color[i][j].r, m_color[i][j].g, m_color[i][j].b);
					glDrawArrays(GL_TRIANGLES, 0, 3*12);
				}
			}
		}

		// Highlight the active square.
		if(indicator % 40 <= 20) {
			glDisable(GL_DEPTH_TEST);
			if(num_copy >= 1) {
				for(int l = num[m_active_x][m_active_z]; l < num[m_active_x][m_active_z] + num_copy; l++)
				{
					updateCube(m_active_x, GLfloat(l), m_active_z);
					glBindVertexArray(m_cube_vao);
					glUniform3f(col_uni, indicator_color.r, indicator_color.g, indicator_color.b);
					glDrawArrays(GL_TRIANGLES, 0, 3*12);
				}
			}
			else {
				updateCube(m_active_x, float(num[m_active_x][m_active_z]), m_active_z);
				glBindVertexArray(m_cube_vao);
				glUniform3f(col_uni, indicator_color.r, indicator_color.g, indicator_color.b);
				glDrawArrays(GL_TRIANGLES, 0, 3*12);
			}
		}
		else {
			indicator_color.r = rand() % 2;
			indicator_color.g = rand() % 2;
			indicator_color.b = rand() % 2;
		}
		indicator++;

	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		if(dragging) {
			if(pre_xpos == 0 && pre_ypos == 0 && cur_xpos == 0 && cur_ypos == 0) {
				pre_xpos = xPos;
				pre_ypos = yPos;
			}
			else {
				cur_xpos = xPos;
				cur_ypos = yPos;
				model = glm::translate(model, glm::vec3((float)DIM/2.0f, 0.0f, (float)DIM/2.0f));
				model = glm::rotate(model, (GLfloat)(cur_xpos - pre_xpos)/100, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::translate(model, glm::vec3(-(float)DIM/2.0f, 0.0f, -(float)DIM/2.0f));
				pre_xpos = cur_xpos;
				pre_ypos = cur_ypos;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if(button == GLFW_MOUSE_BUTTON_LEFT) {
			if (actions == GLFW_PRESS) {
				dragging = true;
				pre_xpos = 0;
				pre_ypos = 0;
				cur_xpos = 0;
				cur_ypos = 0;
			}
			else if(actions == GLFW_RELEASE) {
				dragging = false;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	if(yOffSet > 0 && bound*1.05 <= 2) {
		model = glm::translate(model, glm::vec3((float)DIM/2.0f, 0.0f, (float)DIM/2.0f));
		model = glm::scale(model, glm::vec3(1.05f, 1.05f, 1.05f));
		model = glm::translate(model, glm::vec3(-(float)DIM/2.0f, 0.0f, -(float)DIM/2.0f));
		bound *= 1.05;
	}
	else if(yOffSet < 0 && bound*0.95 >= 0.5) {
		model = glm::translate(model, glm::vec3((float)DIM/2.0f, 0.0f, (float)DIM/2.0f));
		model = glm::scale(model, glm::vec3(0.95f, 0.95f, 0.95f));
		model = glm::translate(model, glm::vec3(-(float)DIM/2.0f, 0.0f, -(float)DIM/2.0f));
		bound *= 0.95;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if(key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if(key == GLFW_KEY_R) {
			reset();
		}

		if(key == GLFW_KEY_SPACE) {
			num[m_active_x][m_active_z]++;
		}

		if(key == GLFW_KEY_BACKSPACE) {
			if(num[m_active_x][m_active_z] > 0)
				num[m_active_x][m_active_z]--;
		}

		if(key == GLFW_KEY_LEFT) {
			if(m_active_x > 0)
				m_active_x--;

			if(is_copy) {
				GLuint x = m_active_x;
				GLuint z = m_active_z;
				num[x][z] = num_copy;
				m_color[x][z] = color_copy;
			}
		}

		if(key == GLFW_KEY_UP) {
			if(m_active_z > 0)
				m_active_z--;
			
			if(is_copy) {
				GLuint x = m_active_x;
				GLuint z = m_active_z;
				num[x][z] = num_copy;
				m_color[x][z] = color_copy;
			}
		}

		if(key == GLFW_KEY_RIGHT) {
			if(m_active_x < DIM - 1)
				m_active_x++;
			
			if(is_copy) {
				GLuint x = m_active_x;
				GLuint z = m_active_z;
				num[x][z] = num_copy;
				m_color[x][z] = color_copy;
			}
		}

		if(key == GLFW_KEY_DOWN) {
			if(m_active_z < DIM - 1)
				m_active_z++;
			
			if(is_copy) {
				GLuint x = m_active_x;
				GLuint z = m_active_z;
				num[x][z] = num_copy;
				m_color[x][z] = color_copy;
			}
		}

		if(key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
			GLuint x = m_active_x;
			GLuint z = m_active_z;
			num_copy = num[x][z];
			color_copy = m_color[x][z];
			is_copy = true;
		}
	}

	if(action == GLFW_RELEASE) {
		if(key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
			num_copy = 1;
			is_copy = false;
		}
	}

	return eventHandled;
}
