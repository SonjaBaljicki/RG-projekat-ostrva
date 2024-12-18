#version 330 core

layout(location = 0) in vec2 position;

uniform vec2 offset;   
uniform float waterLevel; 

out vec2 fragPos;     


void main()
{
    vec2 pos = position;
    if (pos.y < waterLevel) {
        float difference = waterLevel - pos.y; 
        pos.y += difference * 0.4;
        }

    gl_Position = vec4(pos + offset, 0.0, 1.0);
    fragPos=position;
}
