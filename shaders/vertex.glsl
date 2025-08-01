#version 330 core
layout (location = 0) in vec2 aPos;    // Position
layout (location = 1) in vec4 aColor;  // Color

out vec4 vColor;  // Pass color to fragment shader

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    vColor = aColor;
}