#version 330 core

uniform vec2 mousePos;
uniform float mouseDist;
uniform float mouseDistSqr;
uniform vec2 displayBounds;
uniform vec4 mouseBarrierColor;

in vec4 vColor;
out vec4 FragColor;

vec4 over(vec4 top, vec4 bottom) {
    if (top.a == 0.0) {
        return bottom;
    }
    float outAlpha = top.a + bottom.a * (1.0 - top.a);
    vec3 outColor = (top.rgb * top.a + bottom.rgb * bottom.a * (1.0 - top.a)) / outAlpha;
    return vec4(outColor, outAlpha);
}

void main() {
    vec2 fragPos = gl_FragCoord.xy;
    vec2 correctedMousePos = vec2(mousePos.x, displayBounds.y - mousePos.y);
    vec2 diff = fragPos - correctedMousePos;

    if (abs(diff.x) > mouseDist || abs(diff.y) > mouseDist) {
        FragColor = vColor;
        return;
    }

    if (dot(diff, diff) < mouseDistSqr) {
        FragColor = over(mouseBarrierColor, vColor);
    } else {
        FragColor = vColor;
    }
}
