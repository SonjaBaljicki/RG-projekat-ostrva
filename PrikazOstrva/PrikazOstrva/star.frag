#version 330 core
out vec4 FragColor;

uniform float time;          // Trenutno vreme za animaciju
uniform vec3 skyColor;       // Boja neba (za blend)

void main() {
    float alpha = abs(sin(time + gl_FragCoord.x * 0.05)); // Treperenje na osnovu vremena
    vec3 starColor = mix(skyColor, vec3(1.0, 1.0, 1.0), alpha); // Blend sa bojom neba
    FragColor = vec4(starColor, 1.0);
}
