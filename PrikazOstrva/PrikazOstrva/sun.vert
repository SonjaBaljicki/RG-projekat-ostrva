#version 330 core
layout(location = 0) in vec2 position; // Pozicija svakog verteksa
uniform vec2 offset;  // Uniform za pomeranje

void main() {
    gl_Position = vec4(position.x + offset.x, position.y + offset.y, 0.0, 1.0);  // Pomeri poziciju sunca na osnovu offseta
}
