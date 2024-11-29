#version 330 core
out vec4 FragColor;

in vec2 chTex;

uniform sampler2D textTexture;  // Tekstura sa slovima
uniform sampler2D smokeTexture; // Tekstura sa dimom

void main() {
    // Uzimanje boja sa tekstura
    vec4 textColor = texture(textTexture, chTex);
    vec4 smokeColor = texture(smokeTexture, chTex);

    FragColor = mix(textColor, smokeColor, smokeColor.a * 0.5);
}
