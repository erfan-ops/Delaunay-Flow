#version 330 core
in vec4 vColor;     // Color from vertex shader
out vec4 FragColor; // Output color

void main() {
    FragColor = vColor;
}