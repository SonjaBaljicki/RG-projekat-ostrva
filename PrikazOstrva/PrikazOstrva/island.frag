#version 330 core

uniform vec4 color;              
out vec4 FragColor;             
uniform vec4 ambientLight;      

uniform vec2 flameLightPosition;
uniform vec3 flameLightColor;
uniform float flameLightIntensity;

uniform float scaleY;           
in vec2 fragPos;               

void main()
{
    vec4 ambient = ambientLight * color;

    vec2 lightDir = normalize(flameLightPosition - fragPos);
    float diff = max(dot(lightDir, vec2(0.0, 1.0)), 0.0);

    float distance = length(flameLightPosition - fragPos);
    float attenuation = 1.0 / (distance * distance);

    attenuation *= 1.0f + scaleY * 0.4f;  

    vec3 diffuse = diff * flameLightColor * flameLightIntensity * attenuation;

    FragColor = vec4(ambient.rgb + diffuse, 1.0);
}
