#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec3 vColor;
out vec3 fColor; // Sent to fragmentSahder
out vec3 fPosition; // Sent to fragmentShader
out vec3 fNormal; // Sent to fragmentShader

uniform mat4 viewMat, projMat;

void main() {
        gl_Position = projMat * viewMat * vec4(vPosition, 1.0); // mandatory to rasterize properly

        fPosition = vPosition;
        fNormal = vNormal;
        fColor = vColor;
}
