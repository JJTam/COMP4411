varying vec3 normal;
varying vec3 pos;

void main()
{
    normal = normalize(gl_NormalMatrix * gl_Normal);
    pos = gl_ModelViewMatrix * gl_Vertex;
    
    gl_Position = ftransform();
}
