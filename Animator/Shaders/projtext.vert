uniform mat4 projMatrix;
uniform mat4 viewInv;

varying vec3 normal;
varying vec3 pos;
varying vec4 projTexCoord;

void main()
{
    normal = normalize(gl_NormalMatrix * gl_Normal);
    pos = gl_ModelViewMatrix * gl_Vertex;
    // projTexCoord = projMatrix * viewInv * gl_ModelViewMatrix * gl_Vertex;
    projTexCoord = gl_TextureMatrix[0] * gl_ModelViewMatrix * gl_Vertex;
    gl_Position = ftransform();
}
