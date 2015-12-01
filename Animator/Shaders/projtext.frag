#version 120

uniform sampler2D textureSampler;
uniform sampler2D shadowSampler;
uniform int useShadow;

varying vec4 normal;
varying vec4 pos;
varying vec4 projTexCoord;
varying vec4 projPosW;
varying vec4 shadowCoord;

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
        vec4 R = -normalize(L - 2.0*dot(L,normal)*normal);
        vec4 E = normalize(-pos);
        vec4 Ispec = gl_FrontLightProduct[0].specular * pow(max(dot(R,E), 0.0), 0.3 * gl_FrontMaterial.shininess);
        Ispec = clamp(Ispec, 0.0, 1.0);
        
        color = color + NdotL * gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse * distAtten + Ispec;
    }
    
    vec4 projTexColor = vec4(0.0);
    vec4 projDir = normalize(projPosW - pos);
    float projDot = max(dot(normal, projDir), 0.0);
    vec4 projTexCoordDiv = projTexCoord / projTexCoord.q;
    if (projDot > 0.0 && projTexCoord.q > 0.0 && projTexCoordDiv.x > 0.0 && projTexCoordDiv.y > 0.0 && projTexCoordDiv.x < 1.0 && projTexCoordDiv.y < 1.0)
    {
        projTexColor = texture2DProj(textureSampler, projTexCoordDiv);
    }
    else
    {
        projTexColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    
    vec4 shadowCoordWdiv = shadowCoord / shadowCoord.q;
    if (useShadow == 1 && shadowCoordWdiv.x > 0.0 && shadowCoordWdiv.y > 0.0 && shadowCoordWdiv.x < 1.0 && shadowCoordWdiv.y < 1.0)
    {
        shadowCoordWdiv.z -= 1e-4;
        float distFromProj = texture2D(shadowSampler, shadowCoordWdiv.xy).z;
        if (shadowCoord.q > 0.0 && distFromProj < shadowCoordWdiv.z)
            projTexColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    
    gl_FragColor = (color + gl_FrontMaterial.ambient) * projTexColor;
}
