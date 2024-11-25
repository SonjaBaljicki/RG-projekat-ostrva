#version 330 core

uniform vec4 color; // Boja

out vec4 FragColor;

void main()
{
    FragColor = vec4(color);
}