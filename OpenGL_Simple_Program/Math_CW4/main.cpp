#include <Tutorial1PCH.h>
#include <Camera.h>

//Macros Used for the vertex buffer
#define BUFFER_OFFSET(offset) ((void*)(offset))
#define MEMBER_OFFSET(s,m) ((char*)NULL + (offsetof(s,m)))

//Global variables used in our application

//Height, width and ID of the window
int g_iWindowWidth = 800;
int g_iWindowHeight = 600;
int g_iWindowHandle = 0;

//Set to 1 if WASD keys are pressed, 0 otherwise
int g_W, g_A, g_S, g_D, g_Q, g_E;

//Will be set to true if the Shift buttons are pressed, false otherwise
bool g_bShift = false;

//Keep track of the mouse position
glm::ivec2 g_MousePos;

//Store the initial rotation of the object
glm::quat g_Rotation;

//To calculate the amount of time elapsed between frames
std::clock_t g_PreviousTicks;
std::clock_t g_CurrentTicks;

//Camera object and a few parameters to reset it to default position
Camera g_Camera;
glm::vec3 g_InitialCameraPosition;
glm::quat g_InitialCameraRotation;

//Define Geometry of the object. Can be done with a model loader (not in this project). It will be done in-line using static arrays

//Creation of a single vertex: Position and color (Vec3)
struct VertexXYZColor
{
	glm::vec3 m_Pos; //x,y,z position of the vertex
	glm::vec3 m_Color; //RGB values NO Alpha
};

// Define the 8 vertices of a unit cube. The order of vertices in the vertex array is arbitrary
VertexXYZColor g_Vertices[8] = {
	{ glm::vec3(1,  1,  1), glm::vec3(1, 1, 1) },  // 0
	{ glm::vec3(-1,  1,  1), glm::vec3(0, 1, 1) }, // 1
	{ glm::vec3(-1, -1,  1), glm::vec3(0, 0, 1) }, // 2
	{ glm::vec3(1, -1,  1), glm::vec3(1, 0, 1) },  // 3
	{ glm::vec3(1, -1, -1), glm::vec3(1, 0, 0) },  // 4
	{ glm::vec3(-1, -1, -1), glm::vec3(0, 0, 0) }, // 5
	{ glm::vec3(-1,  1, -1), glm::vec3(0, 1, 0) }, // 6
	{ glm::vec3(1,  1, -1), glm::vec3(1, 1, 0) },  // 7
};

//The order to send the vertices to the GPU is determined by the index buffer
// Define the vertex indices for the cube.
// Each set of 6 vertices represents a set of triangles in 
// counter-clockwise winding order.

GLuint g_Indices[36] = {
	0, 1, 2, 2, 3, 0,           // Front face
	7, 4, 5, 5, 6, 7,           // Back face
	6, 5, 2, 2, 1, 6,           // Left face
	7, 0, 3, 3, 4, 7,           // Right face
	7, 6, 1, 1, 0, 7,           // Top face
	3, 2, 5, 5, 4, 3            // Bottom face
};

//Handles to the objects created by OpenGL

// Vertex array object for the cube.
GLuint g_vaoCube = 0;         //Used to refer to the Vertex array object used to render our cube. It binds all the vertex attributes and and the index buffer into a single argument
GLuint g_ShaderProgram = 0;   //Reference to the compiled and linked shader program. The shader program combines both vertex and fragment shader into a single program that after compilation can be run on the GPU.

// Model, View, Projection matrix uniform variable in shader program.
// The MVP acronym suggests that the shader variable defines the concatentated model-view-projection matrix that is used to transform the cube’s vertices into clip-space 
GLint g_uniformMVP = -1;

//Functions used as callbacks for window events
void IdleGL();                                     // called whenever no other windowing events need to be handled. We can use this function to update the “game” logic.
void DisplayGL();                                  // called whenever the contents of the window need to be redrawn (for example, if something in the scene moves or the window is resized).
void KeyboardGL(unsigned char c, int x, int y);    // called if a key is pressed on the keyboard. This function will be called repeatedly if the key is held down.
void KeyboardUpGL(unsigned char c, int x, int y);  // called when a key is released on the keyboard.
void SpecialGL(int key, int x, int y);             // called when a non-printable character (like the shift, control, alt, or arrow keys) is pressed on the keyboard. This function will be called repeatedly if the key is held down.
void SpecialUpGL(int key, int x, int y);           // called when a non-printable character key is released.
void MouseGL(int button, int state, int x, int y); // called when a mouse button is pressed while the mouse cursor is over the window.
void MotionGL(int x, int y);                       // called when the mouse is dragged over the window. We will use this function to rotate the cube.
void ReshapeGL(int w, int h);                      // called when the window is resized.

												   /**
												   * Initialize the OpenGL context and create a render window.
												   */
void InitGL(int argc, char* argv[])
{
	std::cout << "Initialize OpenGL..." << std::endl;

	glutInit(&argc, argv); //Used to initialize some window properties from the command line (http://www.opengl.org/documentation/specs/glut/spec3/node10.html.)

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS); //Options for glut main loop (http://freeglut.sourceforge.net/docs/api.php#StateSetting.)

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH); //Query the window size so OpenGL can center the render window
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH); //How the render window is created: Use RGBA, Alpha , DOuble Buffer and Depth to draw primitives in the correct order

	// Create an OpenGL 3.3 core forward compatible context (not compatible with deprecated stuff).
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	//Window size and position
	glutInitWindowPosition((iScreenWidth - g_iWindowWidth) / 2, (iScreenHeight - g_iWindowHeight) / 2);
	glutInitWindowSize(g_iWindowWidth, g_iWindowHeight);

	//Window is created at this point. That's why always initialize GLUT before Glew so that a window is already there for OpenGL
	g_iWindowHandle = glutCreateWindow("OpenGL Template");

	// Register GLUT callbacks.
	glutIdleFunc(IdleGL);
	glutDisplayFunc(DisplayGL);
	glutKeyboardFunc(KeyboardGL);
	glutKeyboardUpFunc(KeyboardUpGL);
	glutSpecialFunc(SpecialGL);
	glutSpecialUpFunc(SpecialUpGL);
	glutMouseFunc(MouseGL);
	glutMotionFunc(MotionGL);
	glutReshapeFunc(ReshapeGL);

	//Initialize some OpenGL variables suchas depth, color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	std::cout << "Initialize OpenGL Success!" << std::endl;
}

//After we initiliaze OpenGL and created a render window we can initialize GLEW to use the extentions

void InitGLEW()
{
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "There was a problem initializing GLEW. Exiting..." << std::endl;
		exit(-1);
	}

	// Check for 3.3 support.
	// I've specified that a 3.3 forward-compatible context should be created.
	// so this parameter check should always pass if our context creation passed.
	// If we need access to deprecated features of OpenGL, we should check
	// the state of the GL_ARB_compatibility extension.

	if (!GLEW_VERSION_3_3)
	{
		std::cerr << "OpenGL 3.3 required version support not present." << std::endl;
		exit(-1);
	}

#ifdef _WIN32
	if (WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0); // Disable vertical sync
	}
#endif
}

//Loading Shaders

// Loads a shader and returns the compiled shader object.
// If the shader source file could not be opened or compiling the 
// shader fails, then this function returns 0.

GLuint LoadShader(GLenum shaderType, const std::string& shaderFile)
{
	std::ifstream ifs;

	// Load the shader source file.
	ifs.open(shaderFile);

	//Check if the file was successfully opened
	if (!ifs)
	{
		std::cerr << "Can not open shader file: \"" << shaderFile << "\"" << std::endl;
		return 0;
	}

	//If the file was successfully open copy the content to the source string
	std::string source(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
	ifs.close();

	//The next step is to create a new shader object, copy the source code into the shader object and compile it.

	// Create a shader object.
	GLuint shader = glCreateShader(shaderType);

	// Load the shader source for each shader object.
	const GLchar* sources[] = { source.c_str() };
	glShaderSource(shader, 1, sources, NULL);
	glCompileShader(shader);

	// Check for compile errors in the shader
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus); //This will tell if the shadr was compiled
	if (compileStatus != GL_TRUE)
	{
		GLint logLength;                                        //Check the length of the error log
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength); 
		GLchar* infoLog = new GLchar[logLength];                // Create and array with that length
		glGetShaderInfoLog(shader, logLength, NULL, infoLog);  // Create the info Log

#ifdef _WIN32
		//OutputDebugString(infoLog); (Not working, used to send the log directly to the visual studio output log)
		std::cerr << infoLog << std::endl;
#else
		std::cerr << infoLog << std::endl;
#endif
		delete infoLog;
		return 0; //to indicate the shader failed to load
	}

	return shader; //If no errors then the program returns the shader ID
}

//We now need to link all the shader objects created (Vertex and Fragment, loaded previously) to create the shader program
// Create a shader program from a set of compiled shader objects.
GLuint CreateShaderProgram(std::vector<GLuint> shaders)
{
	// Create a shader program.
	GLuint program = glCreateProgram();

	// Attach the appropriate shader objects.
	for (GLuint shader : shaders)
	{
		glAttachShader(program, shader);
	}

	// Link the program
	glLinkProgram(program);

	// Check the link status.
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* infoLog = new GLchar[logLength];

		glGetProgramInfoLog(program, logLength, NULL, infoLog);

#ifdef _WIN32
		//OutputDebugString(infoLog);
		std::cerr << infoLog << std::endl;
#else
		std::cerr << infoLog << std::endl;
#endif

		delete infoLog;
		return 0;
	}

	return program;
}

//We put everything together and create an entry point in the main.

int main(int argc, char* argv[])
{
	g_PreviousTicks = std::clock();
	g_A = g_W = g_S = g_D = g_Q = g_E = 0;

	g_InitialCameraPosition = glm::vec3(0, 0, 10);
	g_Camera.SetPosition(g_InitialCameraPosition);
	g_Camera.SetRotation(g_InitialCameraRotation);

	InitGL(argc, argv);
	InitGLEW();

	// Load some shaders.
	GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, "../data/simpleShader.vert");
	GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "../data/simpleShader.frag");

	std::vector<GLuint> shaders;
	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);

	// Create the shader program.
	g_ShaderProgram = CreateShaderProgram(shaders);
	assert(g_ShaderProgram != 0);

	//if we change the shader program then the attribute locations may change. It is also possible that the linker optimizes unused attributes and uniform variables away if they are not being used anywhere in the final program so it is always a good idea to explicitly request the attribute locations after the shader program has been linked.
	GLint positionAtribID = glGetAttribLocation(g_ShaderProgram, "in_position");
	GLint colorAtribID = glGetAttribLocation(g_ShaderProgram, "in_color");
	g_uniformMVP = glGetUniformLocation(g_ShaderProgram, "MVP");

	// Create a VAO for the cube.First we create a new VAO and bind it to make it active.
	glGenVertexArrays(1, &g_vaoCube);
	glBindVertexArray(g_vaoCube);

	//Next, we’ll create two Vertex Buffer Objects (VBO), one for the vertex data, and another for the index data.
	GLuint vertexBuffer, indexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);

	//Then we need to bind the buffers and populate them with the vertex data.
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_Vertices), g_Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_Indices), g_Indices, GL_STATIC_DRAW);

	//After we have copied the model data to the VBOs, we need to specify which attributes are mapped 
	//to which parts of the VBO. We must specify that the position data in the VBO stream maps to attribute 
	//location 0 and the color data in the VBO stream maps to attribute location 1.

	glVertexAttribPointer(positionAtribID, 3, GL_FLOAT, false, sizeof(VertexXYZColor), MEMBER_OFFSET(VertexXYZColor, m_Pos));
	glEnableVertexAttribArray(positionAtribID);

	glVertexAttribPointer(colorAtribID, 3, GL_FLOAT, false, sizeof(VertexXYZColor), MEMBER_OFFSET(VertexXYZColor, m_Color)); //The glVertexAttribPointer function allows us to map arbitrary vertex attributes to arbitrary attribute locations
	glEnableVertexAttribArray(colorAtribID); //In order for the vertex attributes to be activated in the VAO, we must enable them using the glEnableVertexAttribArray function passing the generic vertex attribute location as the only parameter.

	// Make sure we disable and unbind everything to prevent rendering issues later. we can unbind and disable any states that we have activated during the initialization
	glBindVertexArray(0); //We unbind the currently active VAO and VBO’s by activating the default VAO and VBO 0. This is equivalent to “disabling” the VAO and VBO’s.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(positionAtribID); //We should also disable the generic vertex attributes that were enabled while setting up the VAO.
	glDisableVertexAttribArray(colorAtribID);

	//Kick off the game loop
	glutMainLoop();
}

//Whenever the render window is resized (or sized for the first time when it is 
//created before the first draw call is invoked) the ReshapeGL callback function will be invoked.
//We can use this function to setup our camera parameters correctly based on the size of the render window.

void ReshapeGL(int w, int h)
{
	if (h == 0)
	{
		h = 1;
	}

	g_iWindowWidth = w;
	g_iWindowHeight = h;

	g_Camera.SetViewport(0, 0, w, h);
	g_Camera.SetProjectionRH(60.0f, w / (float)h, 0.1f, 100.0f); //the camera’s projection matrix is initialized with a 60 degree field-of-view, an aspect ratio that matches the aspect ratio of the window, a near clipping plane of 0.1 units and a far clipping plane of 100 units

	glutPostRedisplay(); //tells GLUT to add a PAINT event to the current window’s event queue. This will cause the scene to get redrawn.
}

//The DisplayGL callback function will be invoked whenever the render 
//window needs to be redrawn (which is guaranteed to occur whenever the 
//glutPostRedisplay method is called).

//If the camera moves or the cube rotates, we need to update the MVP matrix. 
//We do this by first computing the combined MVP matrix and assigning this MVP matrix 
//to the uniform variable in the shader using the glUniformMatrix4fv method

void DisplayGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(g_vaoCube);
	glUseProgram(g_ShaderProgram);

	glm::mat4 mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * glm::toMat4(g_Rotation);
	glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, BUFFER_OFFSET(0)); // kick off all of those vertices down the rendering pipeline.

	glUseProgram(0);
	glBindVertexArray(0);

	glutSwapBuffers();
}

//Idle function
//The IdleGL function will run whenever no other events need to be processed on the 
//window’s event queue. Since we don’t really know how much time will pass between calls 
//to the Idle function (this may be different on different computers) we should try to determine 
//the elapsed time each frame and use that to move things smoothly in the scene.

//For this implementation, we are only updating the position of the camera when the user 
//presses the W, A, S, D, Q, and E keys on the keyboard.If so, we will pan the camera at 1 unit / 
//second and if the Shift key is held down, we will move the camera at 5 units / second.

void IdleGL()
{
	g_CurrentTicks = std::clock();
	float deltaTicks = (float)(g_CurrentTicks - g_PreviousTicks);
	g_PreviousTicks = g_CurrentTicks;

	float fDeltaTime = deltaTicks / (float)CLOCKS_PER_SEC;

	float cameraSpeed = 1.0f;
	if (g_bShift)
	{
		cameraSpeed = 5.0f;
	}

	g_Camera.Translate(glm::vec3(g_D - g_A, g_Q - g_E, g_S - g_W) * cameraSpeed * fDeltaTime); //the camera is translated based on the keys that are pressed

	glutPostRedisplay(); //function will ensure the screen is redrawn.

	

}

//The KeyboardGL function will handle events when a key is pressed on the keyboard.

void KeyboardGL(unsigned char c, int x, int y)
{
	switch (c)
	{
	case 'w':
	case 'W':
		g_W = 1;
		break;
	case 'a':
	case 'A':
		g_A = 1;
		break;
	case 's':
	case 'S':
		g_S = 1;
		break;
	case 'd':
	case 'D':
		g_D = 1;
		break;
	case 'q':
	case 'Q':
		g_Q = 1;
		break;
	case 'e':
	case 'E':
		g_E = 1;
		break;
	case 'r':
	case 'R':
		g_Camera.SetPosition(g_InitialCameraPosition);
		g_Camera.SetRotation(g_InitialCameraRotation);
		g_Rotation = glm::quat();
		break;
	case 27:
		glutLeaveMainLoop();
		break;
	}
}

//The KeyboardUpGL function is called when a key is released on the keyboard. In this case, we’ll just reset 
//the state of the appropriate flag when a key is released.

void KeyboardUpGL(unsigned char c, int x, int y)
{
	switch (c)
	{
	case 'w':
	case 'W':
		g_W = 0;
		break;
	case 'a':
	case 'A':
		g_A = 0;
		break;
	case 's':
	case 'S':
		g_S = 0;
		break;
	case 'd':
	case 'D':
		g_D = 0;
		break;
	case 'q':
	case 'Q':
		g_Q = 0;
		break;
	case 'e':
	case 'E':
		g_E = 0;
		break;

	default:
		break;
	}
}

//Next, we’ll handle the special keys (non-printable characters).

void SpecialGL(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_SHIFT_L:
	case GLUT_KEY_SHIFT_R:
	{
		g_bShift = true;
	}
	break;
	}
}

void SpecialUpGL(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_SHIFT_L:
	case GLUT_KEY_SHIFT_R:
	{
		g_bShift = false;
	}
	break;
	}
}

//Mouse Handling
//The MouseGL function gets called when the user presses the mouse button on the screen.
void MouseGL( int button, int state, int x, int y )
{
	g_MousePos = glm::ivec2(x, y);
}

//When the mouse is dragged, the MotionGL function is called.
void MotionGL(int x, int y)
{
	glm::ivec2 mousePos = glm::ivec2(x, y);
	glm::vec2 delta = glm::vec2(mousePos - g_MousePos);
	g_MousePos = mousePos;

	std::cout << "dX: " << delta.x << " dy: " << delta.y << std::endl;

	glm::quat rotX = glm::angleAxis<float>(glm::radians(delta.y) * 0.5f, glm::vec3(1, 0, 0));
	glm::quat rotY = glm::angleAxis<float>(glm::radians(delta.x) * 0.5f, glm::vec3(0, 1, 0));

	//g_Camera.Rotate( rotX * rotY );
	g_Rotation = (rotX * rotY) * g_Rotation;
}




