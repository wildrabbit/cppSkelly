#version 150

uniform mat4 mvp;
in vec2 inPos;
//in vec2 inTexCoord;
in vec4 inColour;
//out vec2 outTexCoord;
out vec4 outColour;
 
void main()
{
    gl_Position = mvp * vec4(inPos.xy, 0.0, 1.0);
  //  outTexCoord = inTexCoord;
	outColour = inColour;
}