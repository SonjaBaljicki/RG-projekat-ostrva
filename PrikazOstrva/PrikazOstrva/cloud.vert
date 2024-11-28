#version 330 core

layout(location = 0) in vec2 aPosition;  // Pozicija svakog vertiksa (kruga)

uniform vec2 offset;    // Pomeranje koje dolazi iz C++ koda (mesto kretanja oblaka)

void main() {
    // Dodajemo pomeranje na poziciju svakog vertiksa
    gl_Position = vec4(aPosition.x + offset.x, aPosition.y + offset.y, 0.0, 1.0);}
