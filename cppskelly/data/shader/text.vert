#version 150

uniform mat4 pvm;
in vec2 inPos;
in vec2 inTexCoord;
out vec2 outTexCoord;
 
void main()
{
    gl_Position = pvm * vec4(inPos.xy, 0.0, 1.0);
    outTexCoord = inTexCoord;
}