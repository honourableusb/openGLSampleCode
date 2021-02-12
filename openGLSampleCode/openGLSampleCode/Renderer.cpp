#include "Renderer.h"

Camera* Renderer::m_camera = new Camera();

nanogui::Screen* Renderer::m_nanogui_screen = nullptr;

/*
 * TODO: Deprecate these
 * 		and use Object::vao and Object::vbo instead for your loaded model
 */
GLuint VBO, VAO;

/*
 * TODO: Deprecate these
 * 		and create global variables for your GUI
 */
enum test_enum {
    Item1 = 0,
    Item2,
    Item3
};
bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string Renderer::model_name = "objs/cube.obj";
test_enum enumval = Item2;
nanogui::Color col_val(1.0f, 1.0f, 1.0f, 1.0f);
bool reset = true;
bool reload_model = false;


// Pre-defined
bool Renderer::keys[1024];
enum render_type {
	POINT = 0,
	LINE = 1,
	TRIANGLE = 2
};
enum culling_type {
	CW = 0,
	CCW = 1
};
enum shading_type {
	SMOOTH = 0,
	FLAT = 1,
};
enum depth_type {
	LESS = 0,
	ALWAYS = 1,
};

/*
 * TODO: Remeber to use these variables
 */
render_type render_val = LINE;
culling_type culling_val = CW;
shading_type shading_val = SMOOTH;
depth_type depth_val = LESS;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::run()
{
	init();
	display(this->m_window);
}

void Renderer::init()
{
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

#if defined(__APPLE__)
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	m_camera->init();

	this->m_window = glfwCreateWindow(m_camera->width, m_camera->height, "CS6366", nullptr, nullptr);
	glfwMakeContextCurrent(this->m_window);

	glewExperimental = GL_TRUE;
	glewInit();

	nanogui_init(this->m_window);
}

void Renderer::nanogui_init(GLFWwindow* window)
{
	m_nanogui_screen = new nanogui::Screen();
	m_nanogui_screen->initialize(window, true);

	glViewport(0, 0, m_camera->width, m_camera->height);

	// Create nanogui gui
	nanogui::FormHelper* gui = new nanogui::FormHelper(m_nanogui_screen);
	nanogui::ref<nanogui::Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");

	/*
	 *	TODO: Define your GUI components here 
	 */ 
	bool enabled = true;
	gui->addGroup("Color");
	gui->addVariable("Object Color", col_val)->setColor(col_val);

	gui->addGroup("Position");
	gui->addVariable("X", dvar)->setSpinnable(true);
	gui->addVariable("Y", dvar)->setSpinnable(true);
	gui->addVariable("Z", dvar)->setSpinnable(true);

	gui->addGroup("Rotate");
	gui->addVariable("Rotate value", ivar)->setSpinnable(true);
	gui->addButton("Rotate right+", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
	gui->addButton("Rotate right-", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
	gui->addButton("Rotate up+", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
	gui->addButton("Rotate up-", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
	gui->addButton("Rotate front+", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");
	gui->addButton("Rotate front-", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");

	gui->addGroup("Configuration");
	gui->addVariable("z near", dvar)->setSpinnable(true);
	gui->addVariable("z far", dvar)->setSpinnable(true);
	gui->addVariable("Render type", render_val, enabled)->setItems({ "Point", "Line", "Triangle" });
	gui->addVariable("Culling type", culling_val, enabled)->setItems({ "CW", "CCW" });
	gui->addVariable("Model name", model_name)->setTooltip("Enter the filename for the object you wish to load here");
	gui->addButton("Reload Model", []() { reload_model = true; })->setTooltip("Load a model based on the given path above");
	gui->addButton("Reset", [this]() {scene_reset(); })->setTooltip("Resets the model and camera position");
    
	m_nanogui_screen->setVisible(true);
	m_nanogui_screen->performLayout();

	glfwSetCursorPosCallback(window,
		[](GLFWwindow* window, double x, double y) {
			m_nanogui_screen->cursorPosCallbackEvent(x, y);
		}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow*, int button, int action, int modifiers) {
			m_nanogui_screen->mouseButtonCallbackEvent(button, action, modifiers);
		}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods) {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
			if (key >= 0 && key < 1024)
			{
				if (action == GLFW_PRESS)
					keys[key] = true;
				else if (action == GLFW_RELEASE)
					keys[key] = false;
			}
		}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow*, unsigned int codepoint) {
			m_nanogui_screen->charCallbackEvent(codepoint);
		}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow*, int count, const char** filenames) {
			m_nanogui_screen->dropCallbackEvent(count, filenames);
		}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow*, double x, double y) {
			m_nanogui_screen->scrollCallbackEvent(x, y);
			//m_camera->ProcessMouseScroll(y);
		}
	);

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow*, int width, int height) {
			m_nanogui_screen->resizeCallbackEvent(width, height);
		}
	);
}

void Renderer::display(GLFWwindow* window)
{
	Shader m_shader = Shader("./shader/basic.vert", "./shader/basic.frag");

	// Main frame while loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		/*
		 * TODO: Able to reset scene and model as needed
		 */
		if(reload_model || reset)
		scene_reset();
	
		camera_move();
		
		m_shader.use();
		
		setup_uniform_values(m_shader);

		draw_scene(m_shader);

		m_nanogui_screen->drawWidgets();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return;
}

void Renderer::scene_reset()
{
	reload_model = false;
	reset = false;
	load_models();
	m_camera->reset();
}

void Renderer::load_models()
{
	Object model = Object(model_name);
	/*
	 * TODO: Create Object class and bind VAO & VBO.
	 * 		Here we just use show simple triangle
	 */

	//Removed the colors from the vertices array

	//Original Array is as folows
	// Set up vertex data (and buffer(s)) and attribute pointers
  //  GLfloat vertices[] = {
  //      // Positions         	// Colors
  //      0.5f, -0.5f, 0.0f,   	1.0f, 0.0f, 0.0f,  // Bottom Right
  //      -0.5f, -0.5f, 0.0f,   	1.0f, 0.0f, 0.0f,  // Bottom Left
  //      0.5f,  0.5f, 0.0f,   	1.0f, 0.0f, 0.0f,   // Top right
		//0.5f,  0.5f, 0.0f,      0.0f, 1.0f, 0.0f,
		//-0.5f, 0.5f, 0.0f,      1.0f, 1.0f, 0.0f,
		//-0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f
  //  };

	/*GLfloat vertices[] = {
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};*/
    // GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model.vertices), &vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO

	/*
	* TODO: You can also set Camera parameters here
	*/

}

void Renderer::draw_scene(Shader& shader)
{
	// Set up some basic parameters
	glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	 * TODO: Remember to enable GL_DEPTH_TEST and GL_CULL_FACE
	 */
	

	/*
	 * TODO: Draw object
	 * 		 Here we only draw a traingle
	 */
	Object model(model_name);
	draw_object(shader, model);
}


void Renderer::draw_object(Shader& shader, Object& object)
{
	/*
	 * TODO: Draw object
	 * 		 Here we only draw a traingle
	 */
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Renderer::setup_uniform_values(Shader& shader)
{
	/*
	 * TODO: Define uniforms for your shader
	*/

}

void Renderer::camera_move()
{
	GLfloat current_frame = glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;
	// Camera controls
	if (keys[GLFW_KEY_W])
		m_camera->process_keyboard(FORWARD, delta_time);
	if (keys[GLFW_KEY_S])
		m_camera->process_keyboard(BACKWARD, delta_time);
	if (keys[GLFW_KEY_A])
		m_camera->process_keyboard(LEFT, delta_time);
	if (keys[GLFW_KEY_D])
		m_camera->process_keyboard(RIGHT, delta_time);
	if (keys[GLFW_KEY_Q])
		m_camera->process_keyboard(UP, delta_time);
	if (keys[GLFW_KEY_E])
		m_camera->process_keyboard(DOWN, delta_time);
	if (keys[GLFW_KEY_I])
		m_camera->process_keyboard(ROTATE_X_UP, delta_time);
	if (keys[GLFW_KEY_K])
		m_camera->process_keyboard(ROTATE_X_DOWN, delta_time);
	if (keys[GLFW_KEY_J])
		m_camera->process_keyboard(ROTATE_Y_UP, delta_time);
	if (keys[GLFW_KEY_L])
		m_camera->process_keyboard(ROTATE_Y_DOWN, delta_time);
	if (keys[GLFW_KEY_U])
		m_camera->process_keyboard(ROTATE_Z_UP, delta_time);
	if (keys[GLFW_KEY_O])
		m_camera->process_keyboard(ROTATE_Z_DOWN, delta_time);
}
