#version 330 core

layout (location = 0) in vec2 aPos;  // Pozicija ajkule (3D koordinata)

uniform float waterLevel;  // Nivo vode (pomeraj po Y osi)
uniform float time;        // Vreme za animaciju
uniform float speed;       // Brzina horizontalnog kretanja

void main() {
    vec2 newPos = aPos;

    // Pomeraj ajkule prema nivou vode, slican pomeraju ostrva
    if (newPos.y < waterLevel) {
        float difference = waterLevel - newPos.y; 
        newPos.y += difference * 0.4;  // Pomeraj ajkule u skladu sa nivoom vode
    }

    newPos.x += sin(time * speed + newPos.y) * 0.09;  // Pomera ajkulu levo-desno u zavisnosti od vremena i visine

    // Postavljanje nove pozicije ajkule
    gl_Position = vec4(newPos, 0.0, 1.0);
}
