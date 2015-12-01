uniform mat4 projMatrix;
uniform mat4 viewInv;
uniform vec4 projPos;

varying vec3 normal;
varying vec3 pos;
varying vec4 projTexCoord;
varying vec4 projPosW;
varying vec4 shadowCoord;
void main()
{
    normal = normalize(gl_NormalMatrix * gl_Normal);
    pos = gl_ModelViewMatrix * gl_Vertex;
    projTexCoord = projMatrix * viewInv * gl_ModelViewMatrix * gl_Vertex;
    projPosW = gl_ModelViewMatrix * projPos;
    shadowCoord = gl_TextureMatrix[7] * viewInv * gl_ModelViewMatrix * gl_Vertex;
    gl_Position = ftransform();
}
