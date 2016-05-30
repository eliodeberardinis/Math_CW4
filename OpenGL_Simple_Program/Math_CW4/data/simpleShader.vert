	#version 330 core   //Version of GLSL tu use

	//define the input attributes that are expected to be sent from the application
	// The first part of the variable definition is called the layout qualifier. The layout qualifier determines the attribute index to bind these variables to. In the application, we will bind the vertex position stream to attribute ID 0 and the vertex color stream to attribute ID 1
	// The in modifier indicates that this variable is input data, that is the value of these variables will be sent from the application. Both of these variables are of type vec3

	//Inputs

	layout(location=0) in vec3 in_position;
    layout(location=1) in vec3 in_color;

	//Output (Color Vector)

	out vec4 v2f_color;

	// Model, View, Projection matrix. Single uniform value of the shader. Uniform means it will stay the same at each invocation. Changes to the uniform are not seen outside the scope of the vertex shader program invocation
    uniform mat4 MVP;

	void main()
{
    gl_Position = MVP * vec4(in_position, 1); //Transform the input vertices by the MVP matrix to obtain the clip space. in_position is vec3 so we need to add 1 or 0. 1 if it's a position. 0 if it's a direction so we dont't translate it.
	                                          //gl_Position is the pre-built variable to pass to the fragment shader. So at least we need to assign the in_position to gl_Position
    // Just pass the color through directly.
    v2f_color = vec4(in_color, 1);            //The color is passed as it is. the 1 refers to alpha
}

//If we want to use the clip space in the fragment shader we need to copy gl_position to an explicit variable. We don't need it now.