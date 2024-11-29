#version 330 core

layout(location = 0) in vec2 position;

uniform vec2 offset;    // Pomeraj pozicije
uniform float waterLevel; // Nivo vode

out vec2 fragPos;                  // Pozicija fragmenta za fragment shader


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
