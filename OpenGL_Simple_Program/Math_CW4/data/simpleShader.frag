#version 330 core

// Input color from the vertex program. (Must have exact name and type)
in vec4 v2f_color; //if the flat modifier is added the colors will not be interpolated between vertices. flat must be added to the output of vertex shader as well

//Output: Color of the fragment
layout (location=0) out vec4 out_color; //since we only have one render target (The color buffer) we assign it to locaiton 0.

void main()
{
    out_color = v2f_color;
}

