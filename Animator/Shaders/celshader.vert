#version 120
varying vec4 normal;
varying vec4 pos;

void main()
{
    normal = vec4(normalize(gl_NormalMatrix * gl_Normal),0);
    pos = gl_ModelViewMatrix * gl_Vertex;
    
    gl_Position = ftransform();
}
