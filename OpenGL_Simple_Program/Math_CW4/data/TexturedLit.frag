#version 330 core
 
 //Passed from the vertex shader
in vec4 v2f_positionW; // Position in world space.
in vec4 v2f_normalW; // Surface normal in world space.
in vec2 v2f_texcoord;
 
uniform vec4 EyePosW;   // Eye (camera) position in world space.
uniform vec4 LightPosW; // Light's position in world space. (The sun)
uniform vec4 LightColor; // Light's diffuse and specular contribution. (Assumed the same for simplicity)
 
 //Materials properties
uniform vec4 MaterialEmissive;
uniform vec4 MaterialDiffuse;
uniform vec4 MaterialSpecular;
uniform float MaterialShininess;
 
uniform vec4 Ambient; // Global ambient contribution. (For simplicity, single global ambient term. Generally choose to separate the global, per-light, and material ambient values.)
 
uniform sampler2D diffuseSampler; // single uniform sampler is used to fetch a texel from the primary texture that is associated with the object being rendered.
 
layout (location=0) out vec4 out_color; //The final fragment color which is mapped to the first color buffer in the active framebuffer

void main()
{
    // Compute the emissive term.
    vec4 Emissive = MaterialEmissive;
 
    // Compute the diffuse term.
    vec4 N = normalize( v2f_normalW );
    vec4 L = normalize( LightPosW - v2f_positionW );
    float NdotL = max( dot( N, L ), 0 );
    vec4 Diffuse =  NdotL * LightColor * MaterialDiffuse;
     
    // Compute the specular term.
    vec4 V = normalize( EyePosW - v2f_positionW );
    vec4 H = normalize( L + V );
    vec4 R = reflect( -L, N );
    float RdotV = max( dot( R, V ), 0 ); //Phong
    float NdotH = max( dot( N, H ), 0 ); //Blinn-Phong
    vec4 Specular = pow( RdotV, MaterialShininess ) * LightColor * MaterialSpecular; //Phong, if you want to try Blinn, replace RdotV with NdotH
     
    out_color = ( Emissive + Ambient + Diffuse + Specular ) * texture( diffuseSampler, v2f_texcoord );
}

//This shader only computes the lighting contribution of a single light source. If you want to define more 
//lights, you can simply create a for-loop around the diffuse and specular lighting calculations and change 
//the LightPosW and the LightColor uniform variables to be arrays. The final Diffuse and Specular values will 
//be the sum of all Diffuse and Specular contributions for all lights in the scene.