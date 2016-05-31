#version 330 core

in vec2 texcoord0;
in vec2 texcoord1;

 
uniform sampler2D tex0;
uniform sampler2D tex1;

 
layout (location=0) out vec4 out_color;
 
void main()
{
    out_color = texture( tex0, texcoord0 ) + texture( tex1, texcoord1 );
}
