#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>

//Important to include GLEW before freeGLUT bc freeGLUT includes the OpenGL header files
#define GLEW_STATIC //macro used to link Glew static library
#include <GL/glew.h>
#include <GL/wglew.h> // For wglSwapInterval

#define FREEGLUT_STATIC //macro needed to link to freeGLUT static library
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS //To force GLM to use radians for angles calculations
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>


//Soil
#include <SOIL.h>
//#include <image_DXT.h>
//#include <image_helper.h>
//#include <stb_image_aug.h>
//#include <stbi_DDS_aug.h>
//#include <stbi_DDS_aug_c.h>


