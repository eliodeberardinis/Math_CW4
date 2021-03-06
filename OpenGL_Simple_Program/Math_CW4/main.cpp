#include <Tutorial1PCH.h>
#include <Camera.h>

#define POSITION_ATTRIBUTE 0
#define NORMAL_ATTRIBUTE 2
#define DIFFUSE_ATTRIBUTE 3
#define SPECULAR_ATTRIBUTE 4
#define TEXCOORD0_ATTRIBUTE 8
#define TEXCOORD1_ATTRIBUTE 9
#define TEXCOORD2_ATTRIBUTE 10

//Macros Used for the vertex buffer
#define BUFFER_OFFSET(offset) ((void*)(offset))
#define MEMBER_OFFSET(s,m) ((char*)NULL + (offsetof(s,m)))

//Global variables used in our application
//Variable that decides which project to draw
int ModelType = 1;

//Height, width and ID of the window
int g_iWindowWidth = 1280;
int g_iWindowHeight = 720;
int g_iWindowHandle = 0;

//Set to 1 if WASD keys are pressed, 0 otherwise
int g_W, g_A, g_S, g_D, g_Q, g_E;

//Will be set to true if the Shift buttons are pressed, false otherwise
bool g_bShift = false;

//Keep track of the mouse position
glm::ivec2 g_MousePos;

//Store the initial rotation of a generic object
glm::quat g_Rotation;

//rotation values of the bjects on out scene
float g_fSunRotation = 0.0f;
float g_fEarthRotation = 0.0f;
float g_fEarthRotation2 = 0.0f;
float g_fMoonRotation = 0.0f;
float g_fMoonRotation2 = 0.0f;

//To calculate the amount of time elapsed between frames
std::clock_t g_PreviousTicks;
std::clock_t g_CurrentTicks;

//Camera object and a few parameters to reset it to default position
Camera g_Camera;
glm::vec3 g_InitialCameraPosition;
glm::quat g_InitialCameraRotation;

// Vertex array object for the Sphere. 
GLuint g_vaoSphere = 0; //The VAO object for the sphere

//Reference to the compiled and linked shader program. The shader program combines both vertex and fragment shader into a single program that after compilation can be run on the GPU. (Sphere)
GLuint g_SimpleShaderProgram = 0;
GLuint g_TexturedLitShaderProgram = 0;

// Model, View, Projection matrix uniform variable in shader program.
GLint g_uniformMVP = -1;
GLint g_uniformModelMatrix = -1;
GLint g_uniformEyePosW = -1;

GLint g_uniformColor = -1;

// Light uniform variables.
GLint g_uniformLightPosW = -1;
GLint g_uniformLightColor = -1;
GLint g_uniformAmbient = -1;

// Material properties.
GLint g_uniformMaterialEmissive = -1;
GLint g_uniformMaterialDiffuse = -1;
GLint g_uniformMaterialSpecular = -1;
GLint g_uniformMaterialShininess = -1;

GLuint g_EarthTexture = 0;
GLuint g_MoonTexture = 0;
GLuint g_StarsTextures = 0;
GLuint g_SunTexture = 0;
GLuint g_VenusTexture = 0;
GLuint g_MarsTexture = 0;
GLuint g_JupiterTexture = 0;
GLuint g_NeptuneTexture = 0;

//Functions used as callbacks for window events
void IdleGL();                                     // called whenever no other windowing events need to be handled. We can use this function to update the �game� logic.
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

	//Compile the Shader
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


//The SOIL_load_OGL_texture method can be used to load a texture and generate an OpenGL texture object that can then be used to texture the objects in our scene.
GLuint LoadTexture(const std::string& file)
{
	GLuint textureID = SOIL_load_OGL_texture(file.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0); //the default texture object ID of 0 is bound to the GL_TEXTURE_2D texture target and thus un-binding any previously bound texture object.

	//We also want to specify the texture filtering mode to GL_LINEAR_MIPMAP_LINEAR 
	//for the GL_TEXTURE_MIN_FILTER GL_LINEAR for the GL_TEXTURE_MAG_FILTER. We set the 
	//GL_TEXTURE_WRAP_* to GL_REPEAT to avoid seams appearing in our object.

	return textureID;
}


//Since all of the objects in our scene (the earth, the moon, and the sun) can all be represented 
//by a sphere, we will generate a single sphere using a Vertex Array Object and use it to render each 
//object in the scene.
//will create a procedural sphere that contains the texture coordinates and the vertex normals that
//are necessary to render the objects with lighting and texture.

//The Sphere function will be divided in 3 parts:

//1-Setting up the vertex attributes
//2-Setting up the index buffer
//3-Setting up the Vertex Array Object (VAO)

//First we�ll setup the vertex attributes for the sphere. Each vertex of the sphere will have three attributes: 
//position, normal, and texture coordinate. 
//The sphere will be centered at the origin (0, 0, 0) and have a specific radius. 
//The number of segments along the principal axis of the sphere is determined by the stacks parameter and the 
//number of segments around the circumference of each stack is determined by the slices parameter.
GLuint SolidSphere(float radius, int slices, int stacks)
{
	using namespace glm;
	using namespace std;

	const float pi = 3.1415926535897932384626433832795f;
	const float _2pi = 2.0f * pi;

	//Vertex attributes
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> textureCoords;

	for (int i = 0; i <= stacks; ++i)
	{
		// V texture coordinate.
		float V = i / (float)stacks;
		float phi = V * pi;

		for (int j = 0; j <= slices; ++j)
		{
			// U texture coordinate.
			float U = j / (float)slices;
			float theta = U * _2pi;

			float X = cos(theta) * sin(phi);
			float Y = cos(phi);
			float Z = sin(theta) * sin(phi);

			positions.push_back(vec3(X, Y, Z) * radius);
			normals.push_back(vec3(X, Y, Z));
			textureCoords.push_back(vec2(U, V));
		}
	}

//Now we have the vertex attributes for our sphere but we cannot simply render the vertices in this order
//(unless we only want to render points) because we must pass the sphere geometry to the GPU using triangles, 
//we need to build an index buffer that defines the order in which to send the vertices to the GPU. We must also
//make sure that the ordering of the vertices in each triangle are correctly wound using a counter-clockwise winding 
//order so that the outside of the sphere is not culled when using back-face culling.

// Now generate the index buffer
	vector<GLuint> indicies;

	for (int i = 0; i < slices * stacks + slices; ++i)
	{
		indicies.push_back(i);
		indicies.push_back(i + slices + 1);
		indicies.push_back(i + slices);

		indicies.push_back(i + slices + 1);
		indicies.push_back(i);
		indicies.push_back(i + 1);
	}

//Each face of the sphere consists of 2 triangles. An upper triangle and a lower triangle. 
//Each iteration through this loop will create 2 triangles for each face of our sphere.

//The final step is to create the Vertex Array Object (VAO) that encapsulates the sphere�s vertex attributes and index buffer.
//In this phase, we�ll create a Vertex Array Object (VAO) and bind the three vertex attributes and the index buffer 
//to the VAO for easy rendering of our sphere

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

//In this case, we are using packed arrays for our data. Each vertex attribute has it�s own Vertex Buffer Object (VBO). 
//In order to define our VAO, we need four VBO�s. Three for the vertex attributes and one for the index buffer.

	GLuint vbos[4];
	glGenBuffers(4, vbos);

	//To copy the attributes to the VBO�s we first bind a VBO to the GL_ARRAY_BUFFER target 
	//and copy the data to the VBO using the glBufferData method.
	//We must also bind the generic vertex attributes to the correct buffer using the 
	//glVertexAttribPointer method and enable the generic vertex attribute using the glEnableVertexAttribArray method.

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(POSITION_ATTRIBUTE);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(NORMAL_ATTRIBUTE);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(vec2), textureCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(TEXCOORD0_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(TEXCOORD0_ATTRIBUTE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(GLuint), indicies.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return vao;
}

//We put everything together and create an entry point in the main.

int main(int argc, char* argv[])
{
	g_PreviousTicks = std::clock();
	g_A = g_W = g_S = g_D = g_Q = g_E = 0;

	//for the cube change 100 to 3
	g_InitialCameraPosition = glm::vec3(0, 0, 100);
	g_Camera.SetPosition(g_InitialCameraPosition);
	g_Camera.SetRotation(g_InitialCameraRotation);

	InitGL(argc, argv);
	InitGLEW();

	//Load the textures
	g_EarthTexture = LoadTexture("../data/Textures/earth.dds");
	g_MoonTexture = LoadTexture("../data/Textures/moon.dds");

	g_SunTexture = LoadTexture("../data/Textures/Sun.dds"); 
	g_VenusTexture = LoadTexture("../data/Textures/Venus.dds");
	g_MarsTexture = LoadTexture("../data/Textures/Mars.dds");
	g_JupiterTexture = LoadTexture("../data/Textures/Jupiter.dds");
	g_NeptuneTexture = LoadTexture("../data/Textures/Neptune.dds");

	g_StarsTextures = LoadTexture("../data/Textures/starfield2.png");

	
	//Load the simple basic shaders
	GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, "../data/simpleShader.vert");
	GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "../data/simpleShader.frag");

	std::vector<GLuint> shaders;
	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);

	//Create the shader program
	g_SimpleShaderProgram = CreateShaderProgram(shaders);
	assert(g_SimpleShaderProgram);

	// Retrieve the location of the color uniform variable in the simple shader program.
	g_uniformColor = glGetUniformLocation(g_SimpleShaderProgram, "color");

	//Next we�ll load the textureLit vertex and fragment shaders and query for the uniform locations in that shader program.
	vertexShader = LoadShader(GL_VERTEX_SHADER, "../data/TexturedLit.vert");
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "../data/TexturedLit.frag");

	shaders.clear();

	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);

	g_TexturedLitShaderProgram = CreateShaderProgram(shaders);
	assert(g_TexturedLitShaderProgram);

	g_uniformMVP = glGetUniformLocation(g_TexturedLitShaderProgram, "ModelViewProjectionMatrix");
	g_uniformModelMatrix = glGetUniformLocation(g_TexturedLitShaderProgram, "ModelMatrix");
	g_uniformEyePosW = glGetUniformLocation(g_TexturedLitShaderProgram, "EyePosW");

	// Light properties.
	g_uniformLightPosW = glGetUniformLocation(g_TexturedLitShaderProgram, "LightPosW");
	g_uniformLightColor = glGetUniformLocation(g_TexturedLitShaderProgram, "LightColor");

	// Global ambient.
	g_uniformAmbient = glGetUniformLocation(g_TexturedLitShaderProgram, "Ambient");

	// Material properties.
	g_uniformMaterialEmissive = glGetUniformLocation(g_TexturedLitShaderProgram, "MaterialEmissive");
	g_uniformMaterialDiffuse = glGetUniformLocation(g_TexturedLitShaderProgram, "MaterialDiffuse");
	g_uniformMaterialSpecular = glGetUniformLocation(g_TexturedLitShaderProgram, "MaterialSpecular");
	g_uniformMaterialShininess = glGetUniformLocation(g_TexturedLitShaderProgram, "MaterialShininess");

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
	g_Camera.SetProjectionRH(30.0f, w / (float)h, 0.1f, 2000.0f); //the camera�s projection matrix is initialized with a 60 degree field-of-view, an aspect ratio that matches the aspect ratio of the window, a near clipping plane of 0.1 units and a far clipping plane of 100 units

	glutPostRedisplay(); //tells GLUT to add a PAINT event to the current window�s event queue. This will cause the scene to get redrawn.
}

//The DisplayGL callback function will be invoked whenever the render 
//window needs to be redrawn (which is guaranteed to occur whenever the 
//glutPostRedisplay method is called).

//For the Earth example
//To render this scene, we�ll draw 3 spheres. The first sphere will represent the sun. 
//This will be an unlit white sphere that will rotate about 90,000 Km around the center of the scene. 
//The position of the only light in the scene will be the same as the object that represents the sun.
//The Earth is placed at the center of the scene.The earth rotates around its poles, but its positions 
//stays fixed at the center of the scene(the Earth will not be translated).
//The final object will be the moon.The moon appears to rotate around the earth but at a distance of 60, 000 Km 
//away from the earth.
//For the solar system example 
//the sun will be at the centre of the universe with the planets rotating around it.

void DisplayGL()
{
	if (ModelType == 0) {
		//In the first part of the render function, we will create a sphere VAO that 
		//can be used to render the sun, earth, and moon.

		int slices = 32;
		int stacks = 32;
		int numIndicies = (slices * stacks + slices) * 6;
		if (g_vaoSphere == 0)
		{
			g_vaoSphere = SolidSphere(1, slices, stacks);
		}

		//The next part of the render function will setup the uniform properties in the shader 
		//and render the sun, earth, and moon. Let�s first draw the sun using the SimpleShader 
		//shader program (since the sun is not textured or lit, we do not need to use the TextureLit 
		//shader program for this)

		const glm::vec4 white(1);
		const glm::vec4 black(0);
		const glm::vec4 ambient(0.1f, 0.1f, 0.1f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw the sun using the simple shader
		glBindVertexArray(g_vaoSphere);

		glUseProgram(g_SimpleShaderProgram);
		glm::mat4 modelMatrix = glm::rotate(glm::radians(g_fSunRotation), glm::vec3(0, -1, 0)) * glm::translate(glm::vec3(90, 0, 0));
		glm::mat4 mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;
		GLuint uniformMVP = glGetUniformLocation(g_SimpleShaderProgram, "MVP");
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform4fv(g_uniformColor, 1, glm::value_ptr(white));

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		//Next, we will draw the earth.
		// Draw the earth.
		glBindTexture(GL_TEXTURE_2D, g_EarthTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrix[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(12.756f));
		glm::vec4 eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw the moon.
		glBindTexture(GL_TEXTURE_2D, g_MoonTexture);

		modelMatrix = glm::rotate(glm::radians(g_fMoonRotation), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(60, 0, 0)) * glm::scale(glm::vec3(3.476f));
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 5.0f);


		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw the Stars.
		glBindTexture(GL_TEXTURE_2D, g_StarsTextures);

		modelMatrix = glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(250.0f));
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix; //Transforms vertices into clip space

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(black));
		glUniform1f(g_uniformMaterialShininess, 0.1f);

		//glDisable(GL_CULL_FACE);//Alternative method to see the inside of the sphere with the stars
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		glFrontFace(GL_CCW);
		//glEnable(GL_CULL_FACE);


		//The VAO, shader program, and texture should be unbound so that we leave the OpenGL 
		//state machine the way we found it.

		glBindVertexArray(0);
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glutSwapBuffers();
	}

	else {
	
		//In the first part of the render function, we will create a sphere VAO that 
		//can be used to render the sun, earth, and moon.

		int slices = 32;
		int stacks = 32;
		int numIndicies = (slices * stacks + slices) * 6;
		if (g_vaoSphere == 0)
		{
			g_vaoSphere = SolidSphere(1, slices, stacks);
		}

		//The next part of the render function will setup the uniform properties in the shader 
		//and render the sun, earth, and moon. Let�s first draw the sun using the SimpleShader 
		//shader program (since the sun is not textured or lit, we do not need to use the TextureLit 
		//shader program for this)

		const glm::vec4 white(1);
		const glm::vec4 black(0);
		const glm::vec4 ambient(0.1f, 0.1f, 0.1f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Draw the sun using the simple shader
		glBindVertexArray(g_vaoSphere);
		

		glUseProgram(g_SimpleShaderProgram);
		glm::mat4 modelMatrixSun = glm::rotate(glm::radians(0.0f), glm::vec3(0, -1, 0)) * glm::scale(glm::vec3(12.756f));
		glm::mat4 mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrixSun;
		GLuint uniformMVP = glGetUniformLocation(g_SimpleShaderProgram, "MVP");
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform4fv(g_uniformColor, 1, glm::value_ptr(white));

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		//Draw the sun with textures
		glBindTexture(GL_TEXTURE_2D, g_SunTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrixSun[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		glm::mat4 modelMatrix = glm::rotate(glm::radians(g_fEarthRotation), glm::vec3(0, -1, 0)) * glm::scale(glm::vec3(13.0f));
		glm::vec4 eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		//Next, we will draw the earth.
		// Draw the earth.
		glBindTexture(GL_TEXTURE_2D, g_EarthTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrixSun[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(60, 0, 0)) * glm::rotate(glm::radians(g_fEarthRotation2), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(3.476f));
		eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		//// Draw the moon.
		glBindTexture(GL_TEXTURE_2D, g_MoonTexture);

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(60, 0, 0)) * glm::rotate(glm::radians(g_fMoonRotation2), glm::vec3(0, -1, 0))* glm::translate(glm::vec3(8, 0, 0));
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 5.0f);


		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw Venus.
		glBindTexture(GL_TEXTURE_2D, g_VenusTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrixSun[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation*1.3f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(30, 0, 0)) * glm::rotate(glm::radians(g_fEarthRotation2), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(2.0f));
		eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw Mars.
		glBindTexture(GL_TEXTURE_2D, g_MarsTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrixSun[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation*0.8f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(75, 0, 0)) * glm::rotate(glm::radians(g_fEarthRotation2), glm::vec3(0, -1, 0)) * glm::scale(glm::vec3(2.5f));
		eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw Jupiter.
		glBindTexture(GL_TEXTURE_2D, g_JupiterTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrixSun[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation*1.5f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(100, 0, 0)) * glm::rotate(glm::radians(g_fEarthRotation2), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(5.0f));
		eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw Neptune.
		glBindTexture(GL_TEXTURE_2D, g_NeptuneTexture);
		glUseProgram(g_TexturedLitShaderProgram);

		// Set the light position to the position of the Sun.
		glUniform4fv(g_uniformLightPosW, 1, glm::value_ptr(modelMatrixSun[3]));
		glUniform4fv(g_uniformLightColor, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformAmbient, 1, glm::value_ptr(ambient));

		modelMatrix = glm::rotate(glm::radians(g_fEarthRotation*1.3f), glm::vec3(0, -1, 0)) * glm::translate(glm::vec3(130, 0, 0)) * glm::rotate(glm::radians(g_fEarthRotation2*1.2f), glm::vec3(0, -1, 0)) * glm::scale(glm::vec3(4.2f));
		eyePosW = glm::vec4(g_Camera.GetPosition(), 1);
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix;

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniform4fv(g_uniformEyePosW, 1, glm::value_ptr(eyePosW));

		// Material properties.
		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(white));
		glUniform1f(g_uniformMaterialShininess, 50.0f);

		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Draw the Stars. (WORKING: Stars in a Big sphere around everything)
		glBindTexture(GL_TEXTURE_2D, g_StarsTextures);

		modelMatrix = glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(250.0f));
		mvp = g_Camera.GetProjectionMatrix() * g_Camera.GetViewMatrix() * modelMatrix; //Transforms vertices into clip space

		glUniformMatrix4fv(g_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(g_uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glUniform4fv(g_uniformMaterialEmissive, 1, glm::value_ptr(white));
		glUniform4fv(g_uniformMaterialDiffuse, 1, glm::value_ptr(black));
		glUniform4fv(g_uniformMaterialSpecular, 1, glm::value_ptr(black));
		glUniform1f(g_uniformMaterialShininess, 0.1f);

		//glDisable(GL_CULL_FACE);//Alternative method to see the inside of the sphere with the stars
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		glFrontFace(GL_CCW);
		//glEnable(GL_CULL_FACE);

		//The VAO, shader program, and texture should be unbound so that we leave the OpenGL 
		//state machine the way we found it.

		glBindVertexArray(0);
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glutSwapBuffers();

	}
}

//Idle function
//The IdleGL function will run whenever no other events need to be processed on the 
//window�s event queue. Since we don�t really know how much time will pass between calls 
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
	

	// Rate of rotation in (degrees) per second
	const float fRotationRate1 = 5.0f;
	const float fRotationRate2 = 12.5f;
	const float fRotationRate3 = 20.0f;
	const float fRotationRate4 = 15.0f;
	const float fRotationRate5 = 50.0f;

	g_fEarthRotation += fRotationRate1 * fDeltaTime;
	g_fEarthRotation = fmod(g_fEarthRotation, 360.0f);

	g_fEarthRotation2 += fRotationRate4 * fDeltaTime;
	g_fEarthRotation2 = fmod(g_fEarthRotation2, 360.0f);

	g_fMoonRotation += fRotationRate2 * fDeltaTime;
	g_fMoonRotation = fmod(g_fMoonRotation, 360.0f);

	g_fMoonRotation2 += fRotationRate5 * fDeltaTime;
	g_fMoonRotation2 = fmod(g_fMoonRotation2, 360.0f);

	g_fSunRotation += fRotationRate3 * fDeltaTime;
	g_fSunRotation = fmod(g_fSunRotation, 360.0f);

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

	case 32:
		if (ModelType == 1) {ModelType = 0;}
		else                { ModelType = 1;}
		break;

	case 'r':
	case 'R':
		g_Camera.SetPosition(g_InitialCameraPosition);
		g_Camera.SetRotation(g_InitialCameraRotation);
		g_fEarthRotation = 0.0f;
		g_fSunRotation = 0.0f;
		g_fMoonRotation = 0.0f;
		break;
	case 27:
		glutLeaveMainLoop();
		break;
	}
}

//The KeyboardUpGL function is called when a key is released on the keyboard. In this case, we�ll just reset 
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

//Next, we�ll handle the special keys (non-printable characters).

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

	g_Camera.Rotate( rotX * rotY );
}




