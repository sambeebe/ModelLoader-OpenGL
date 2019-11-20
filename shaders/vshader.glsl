#version 150

in  vec4 vPosition;
in  vec3 vNormal;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 NormalMatrix;

out vec4 pos;
out vec4 N;


void main()
{
    
    // Transform vertex normal into eye coordinates
    N = NormalMatrix*vec4(vNormal, 0.0); N.w = 0.0;
    N = normalize(N);
    
    // Transform vertex position into eye coordinates
    pos = ModelView * vPosition;
    gl_Position = Projection * pos;
    
}

