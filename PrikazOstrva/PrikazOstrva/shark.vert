#version 330 core

layout (location = 0) in vec2 aPos;

uniform float waterLevel; 
uniform float time;     
uniform float speed;       

void main() {
    vec2 newPos = aPos;

    if (newPos.y < waterLevel) {
        float difference = waterLevel - newPos.y; 
        newPos.y += difference * 0.4; 
    }

    newPos.x += sin(time * speed + newPos.y) * 0.09;
    gl_Position = vec4(newPos, 0.0, 1.0);
}
