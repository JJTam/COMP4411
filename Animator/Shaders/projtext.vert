#version 120

uniform mat4 projMatrix;
uniform mat4 viewInv;
uniform vec4 projPos;

varying vec3 normal;
varying vec4 pos;
varying vec4 projTexCoord;
varying vec4 projPosW;
varying vec4 shadowCoord;
varying vec4 glpos;

void main()
{
    normal = normalize(gl_NormalMatrix * gl_Normal);
    pos = gl_ModelViewMatrix * gl_Vertex;
    projTexCoord = projMatrix * viewInv * gl_ModelViewMatrix * gl_Vertex;
    projPosW = gl_ModelViewMatrix * projPos;
    shadowCoord = gl_TextureMatrix[7] * viewInv * gl_ModelViewMatrix * gl_Vertex;
    glpos = ftransform();
    gl_Position = ftransform();
}
