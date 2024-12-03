#version 330 core
out vec4 FragColor;

uniform float time;         
uniform vec3 skyColor;      

void main() {
    float alpha = abs(sin(time + gl_FragCoord.x * 0.05));
    vec3 starColor = mix(skyColor, vec3(1.0, 1.0, 1.0), alpha); 
    FragColor = vec4(starColor, 1.0);
}
