#version 330 core

uniform vec4 color;

out vec4 FragColor;
uniform vec4 ambientLight; 


void main()
{
    vec4 ambient = ambientLight * color;
    FragColor = vec4(ambient);
}