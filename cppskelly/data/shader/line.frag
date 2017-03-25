#version 450
precision highp float;                            

//in vec2 outTexCoord;     
in vec4 outColour;
//uniform sampler2D texture;
out vec4 fragColour;           

void main()                                         
{
	fragColour = outColour;
}