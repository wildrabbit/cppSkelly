#version 450
precision mediump float;                            

in vec2 outTexCoord;     

uniform sampler2D texture;
out vec4 fragColour;           

void main()                                         
{
	//vec2 size = vec2(256,256);
	//int samples = 5;
	//float quality = 2.5;
 //   vec4 colour = vec4(0.5, 0.5, 1.0, 0.8);               
                                                   
  fragColour = texture2D(texture, outTexCoord);
  //vec4 sum = vec4(0,0,0,0);
  //int diff = (samples - 1) / 2;
  //vec2 sizeFactor = vec2(1) / size * quality;
  //for (int x = -diff; x <= diff; ++x)
  //{
  //  for (int y = -diff; y <= diff; y++)
  //  {
  //    vec2 offset = vec2(x, y) * sizeFactor;
  //    sum += texture2D(texture, outTexCoord + offset);
  //  }
  //}
  ////fragColour = vec4(1.0,0.0,0.0,1.0);

  //fragColour = ((sum/(samples * samples)) + fragColour) * colour;
  ////return fragColour;
}