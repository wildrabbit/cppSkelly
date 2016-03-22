#version 150

in vec4 inPos;
in vec2 inTexCoord;
out vec2 outTexCoord;
 
void main()
{
    gl_Position = inPos;
    outTexCoord = inTexCoord;
}