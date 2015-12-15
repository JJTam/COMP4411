#version 120
varying vec4 normal;
varying vec4 pos;

void main (void)  
{  
    vec4 color = vec4(0, 0, 0, 0);

    for (int i = 0; i < 2; ++i)
    {
        vec4 lightDir = normalize(gl_LightSource[i].position);
        float NdotL = max(dot(normal, lightDir), 0.0);
        float dist = distance(gl_LightSource[i].position, pos);

        float distAtten = 0.3 / (0.25 + 0.0033 * dist + 0.000045492 * dist * dist);
        
        vec4 L = normalize(gl_LightSource[i].position - pos);
        vec4 R = normalize(-reflect(L, normal));
        vec4 E = normalize(-pos);
        vec4 Ispec = gl_FrontLightProduct[0].specular * pow(max(dot(R,E), 0.0), 0.3 * gl_FrontMaterial.shininess);
        Ispec = clamp(Ispec, 0.0, 1.0);
        
        color = color + NdotL * gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse * distAtten + Ispec;
    }
    
    gl_FragColor = color + gl_FrontMaterial.ambient;
}
