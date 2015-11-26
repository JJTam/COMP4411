
void main()
{
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    vec4 color = vec4(0, 0, 0, 0);
    
    for (int i = 0; i < 2; ++i)
    {
        vec3 lightDir = normalize(vec3(gl_LightSource[i].position));
        float NdotL = max(dot(normal, lightDir), 0.0);
        color = color + NdotL * gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse;
    }

    gl_FrontColor = color;
    gl_Position = ftransform();
}
