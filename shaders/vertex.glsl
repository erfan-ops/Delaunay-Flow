#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform float aspectRatio;

out vec4 vColor;

void main() {
    gl_Position = vec4(aPos.x / aspectRatio, aPos.y, 0.0, 1.0);
    vColor = aColor;
}