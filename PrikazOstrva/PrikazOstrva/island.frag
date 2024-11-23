#version 330 core

uniform vec3 color; // Boja

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1.0);
}