#version 410 core

in vec3 fPos;
in vec3 fNorm;
out vec4 fColor;

// WORLD COORDS
uniform vec3 lightPosition;

uniform vec3 ambientLight;  
uniform vec3 diffuseLight;  
uniform float shininess;

uniform vec3 viewPos;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;

in vec2 fTexCoord;
uniform sampler2D textureMap;

// CAR LIGHTS BASED ON LEARN OPEN GL
uniform int carLightsOn;
uniform vec3 carLightsPos[2];
uniform vec3 carLightsDir;
uniform float inner;
uniform float outer;
uniform vec3 carLightsDiffuse;
uniform vec3 carLightsSpecular;
uniform float Constant;
uniform float Linear;
uniform float Quadratic;

// LAMP LIGHTS
uniform int numLamps;
uniform vec3 lampPositions[30]; 
uniform vec3 lampColor; 
uniform vec3 lampDir; 
uniform float lampInner; 
uniform float lampOuter;

float toon(float v)
{
    if (v > 0.8) return 1.0;
    if (v > 0.6) return 0.8;
    if (v > 0.4) return 0.6;
    if (v > 0.2) return 0.4;
    return 0.2;
}

void main(void)
{
    // TEXTURE COLOR
    vec4 texColor = texture(textureMap, fTexCoord);

    // CALCULATION OF AMBIENT LAYER
    vec3 ambientLayer = ambientColor.rgb * ambientLight;

    // CALCULATION OF DIFFUSE LAYER
    vec3 N = normalize(fNorm);
    vec3 L = normalize(lightPosition - fPos);
    vec3 V = normalize(viewPos - fPos);

    float alignmentDiffuse = toon(max(dot(N, L), 0.0));

    vec3 diffuseLayer = alignmentDiffuse * diffuseColor.rgb * diffuseLight * texColor.rgb;

    // CALCULATION OF SPECULAR LAYER (removed in cartoon, kept as ambient only)
    vec3 finalColor = (ambientLayer + diffuseLayer);

    // CALCULATION OF CAR LIGHTS
    if (carLightsOn == 1)
    {
        for (int i = 0; i < 2; i++)
        {
            vec3 fragToCarLight = normalize(carLightsPos[i] - fPos);
            float theta         = dot(fragToCarLight, normalize(-carLightsDir));
            float intensity     = clamp((theta - outer) / (inner - outer), 0.0, 1.0);

            float dist        = length(carLightsPos[i] - fPos);
            float attenuation = 1.0 / (Constant + Linear * dist + Quadratic * dist * dist);

            float diffuse        = toon(max(dot(N, fragToCarLight), 0.0));
            vec3 diffuseColorCar = diffuse * carLightsDiffuse * diffuseColor.rgb * texColor.rgb;

            vec3 reflection       = reflect(-fragToCarLight, N);
            float specular        = toon(max(dot(V, reflection), 0.0));
            vec3 specularColorCar = specular * carLightsSpecular * specularColor.rgb;

            finalColor += (diffuseColorCar + specularColorCar) * intensity * attenuation;
        }
    }

    
    // CALCULATION OF LAMP LIGHTS
    for(int i = 0; i < numLamps; i++)
    {
        vec3 fragToLamp = lampPositions[i] - fPos;
        float dist = length(fragToLamp);
        fragToLamp = normalize(fragToLamp);
        
        float theta = dot(fragToLamp, normalize(-lampDir));
        float intensity = clamp((theta - lampOuter) / (lampInner - lampOuter), 0.0, 1.0);
        
        // Si intensity es 0.0 (está fuera del cono de luz), nos podríamos ahorrar el cálculo, 
        // pero lo multiplicaremos al final.
        
        float attenuation = 1.0 / (Constant + Linear * dist + Quadratic * dist * dist);
        
        float diff = max(dot(N, fragToLamp), 0.0);
        vec3 diffuseLamp = diff * lampColor * diffuseColor.rgb * texColor.rgb;
        
        vec3 reflection = reflect(-fragToLamp, N);
        float spec = pow(max(dot(V, reflection), 0.0), shininess);
        vec3 specularLamp = spec * lampColor * specularColor.rgb;
        
        // Multiplicamos también por la INTENSIDAD del cono
        finalColor += (diffuseLamp + specularLamp) * attenuation * intensity;
    }

    fColor = vec4(finalColor, 1.0);
}