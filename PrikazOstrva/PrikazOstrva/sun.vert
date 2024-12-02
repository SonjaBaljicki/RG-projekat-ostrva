#version 330 core
layout(location = 0) in vec2 position;
uniform vec2 offset;

void main() {
    gl_Position = vec4(position.x + offset.x, position.y + offset.y, 0.0, 1.0);
}
