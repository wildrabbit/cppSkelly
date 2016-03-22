#version 450
precision mediump float;                            

in vec2 outTexCoord;     
uniform sampler2D texture;
out vec4 fragColour;                              
void main()                                         
{                                                   
  fragColour = texture2D(texture, outTexCoord);
}