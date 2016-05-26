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
