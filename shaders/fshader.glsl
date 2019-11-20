#version 150

uniform vec4 AmbientProduct;
uniform vec4 DiffuseProduct;
uniform vec4 SpecularProduct;
uniform vec4 LightPosition;
uniform float Shininess;

in vec4 pos;
in vec4 N;

out vec4 fragColor;

void main()
{
  
  // Ambient
  vec4 ambient = AmbientProduct;
 
  // Diffuse
  // V = -pos
  // V.w = 0.0
  float D  = .505;
  vec4 V = normalize(-1. * pos);
  V.w = 0.0;
  
//  vec4 Nn = normalize(N);
  vec4 L = normalize(LightPosition - pos) ;
  vec4 R = normalize(-reflect(L,N));
  vec4 diffuse =  DiffuseProduct * max(dot(N, L), 0.);

  //Specular
  float S = 0.0;
  vec4  specular = SpecularProduct * pow(max(dot(R,V), 0.5), Shininess );
  // vec4  specular = SpecularProduct * pow(max(dot(R,V), 0.5), 10.0 );
  fragColor = ambient + diffuse + specular;

//    fragColor = N;
}

