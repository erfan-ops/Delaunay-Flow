#pragma once

const char* vertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform float aspectRatio;

out vec4 vColor;

void main() {
    gl_Position = vec4(aPos.x / aspectRatio, aPos.y, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShader = R"(
#version 330 core

uniform vec2 mousePos;
uniform vec2 displayBounds;
uniform float mouseBarrierRadius;
uniform vec4 mouseBarrierColor;
uniform float mouseBarrierBlur;

in vec4 vColor;
out vec4 FragColor;

vec4 over(vec4 top, vec4 bottom) {
    if (top.a == 0.0) return bottom;
    float outAlpha = top.a + bottom.a * (1.0 - top.a);
    vec3 outColor = (top.rgb * top.a + bottom.rgb * bottom.a * (1.0 - top.a)) / outAlpha;
    return vec4(outColor, outAlpha);
}

void main() {
    vec2 fragPos = gl_FragCoord.xy;
    vec2 correctedMousePos = vec2(mousePos.x, displayBounds.y - mousePos.y);
    vec2 diff = fragPos - correctedMousePos;

    float dist = length(diff);
    float edge = mouseBarrierRadius;

    float aa = fwidth(dist) * mouseBarrierBlur;

    float alpha = smoothstep(edge + aa, edge - aa, dist);

    vec4 color = vec4(mouseBarrierColor.rgb, mouseBarrierColor.a * alpha);
    FragColor = over(color, vColor);
}

)";