#version 330 core

layout(location = 0) in vec2 aPosition;

uniform vec2 offset; 

void main() {
    gl_Position = vec4(aPosition.x + offset.x, aPosition.y + offset.y, 0.0, 1.0);}
