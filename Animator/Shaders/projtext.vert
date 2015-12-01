#version 120

uniform mat4 projMatrix;
uniform mat4 viewInv;
uniform vec4 projPos;

varying vec4 normal;
varying vec4 pos;
varying vec4 projTexCoord;
varying vec4 projPosW;
varying vec4 shadowCoord;

void main()
{
    normal = vec4(normalize(gl_NormalMatrix * gl_Normal),0);
    pos = gl_ModelViewMatrix * gl_Vertex;
    projTexCoord = projMatrix * viewInv * gl_ModelViewMatrix * gl_Vertex;
    projPosW = gl_ModelViewMatrix * projPos;
    shadowCoord = projTexCoord;
    gl_Position = ftransform();
}
