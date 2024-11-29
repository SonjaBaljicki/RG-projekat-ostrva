#version 330 core

uniform vec4 color;              // Boja plamena
out vec4 FragColor;              // Izlazna boja fragmenta
uniform vec4 ambientLight;       // Ambijentalna svetlost

// Parametri svetla
uniform vec3 flameLightPosition; // Pozicija svetla
uniform vec3 flameLightColor;    // Boja svetla
uniform float flameLightIntensity; // Intenzitet svetla

uniform float scaleY;            // Faktor skaliranja za plamen (visina plamena)
in vec2 fragPos;                // Pozicija fragmenta

void main()
{
    vec4 ambient = ambientLight * color;

    vec2 lightDir = normalize(flameLightPosition.xy - fragPos);
    float diff = max(dot(lightDir, vec2(0.0, 1.0)), 0.0);

    float distance = length(flameLightPosition.xy - fragPos);
    float attenuation = 1.0 / (distance * distance);

    attenuation *= 1.0f + scaleY * 0.4f;  

    vec3 diffuse = diff * flameLightColor * flameLightIntensity * attenuation;

    FragColor = vec4(ambient.rgb + diffuse, 1.0);
}
