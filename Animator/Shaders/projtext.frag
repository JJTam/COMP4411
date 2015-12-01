uniform sampler2D textureSampler;
uniform sampler2D shadowSampler;
uniform int useShadow;

varying vec3 normal;
varying vec3 pos;
varying vec4 projTexCoord;
varying vec4 projPosW;
varying vec4 shadowCoord;

void main (void)  
{  
    vec4 color = vec4(0, 0, 0, 0);

    for (int i = 0; i < 2; ++i)
    {
        vec3 lightDir = normalize(vec3(gl_LightSource[i].position));
        float NdotL = max(dot(normal, lightDir), 0.0);
        float dist = distance(gl_LightSource[i].position, pos);

        float distAtten = 0.3 / (0.25 + 0.0033 * dist + 0.000045492 * dist * dist);
        
        vec3 L = normalize(gl_LightSource[i].position - pos);
        vec3 R = normalize(-reflect(L, normal));
        vec3 E = normalize(-pos);
        vec4 Ispec = gl_FrontLightProduct[0].specular * pow(max(dot(R,E), 0.0), 0.3 * gl_FrontMaterial.shininess);
        Ispec = clamp(Ispec, 0.0, 1.0);
        
        color = color + NdotL * gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse * distAtten + Ispec;
    }
    
    vec4 projTexColor = vec4(0.0);
    vec3 projDir = normalize(projPosW - pos);
    float projDot = max(dot(normal, projDir), 0.0);
    projTexCoord = projTexCoord / projTexCoord.q;
    if (projDot > 0 && projTexCoord.q > 0 && projTexCoord.x > 0 && projTexCoord.y > 0 && projTexCoord.x < 1 && projTexCoord.y < 1)
    {
        projTexColor = texture2DProj(textureSampler, projTexCoord);
    }
    else
    {
        projTexColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    
    if (useShadow == 1)
    {
        vec4 shadowCoordWdiv = shadowCoord / shadowCoord.w;
        shadowCoordWdiv.z += 0.0005;
        float distFromProj = texture2D(shadowSampler, shadowCoordWdiv.xy).z;
        if (shadowCoord.w > 0 && distFromProj < shadowCoordWdiv.z)
            projTexColor = vec4(0.5, 0.5, 0.5, 0.5);
    }
    
    gl_FragColor = (color + gl_FrontMaterial.ambient) * projTexColor;
}
