#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec3 vLight;
layout(location=3) in vec3 vAmbient;
layout(location=4) in vec2 vTexCoord;

// Sent to fragmentShader
out vec3 fPosition;
out vec3 fNormal; 
out vec3 fLight;
out vec3 fAmbient;
out vec2 fTexCoord;

uniform mat4 viewMat, projMat;

void main() {
        gl_Position = projMat * viewMat * vec4(vPosition, 1.0); // mandatory to rasterize properly

        fPosition = vPosition;
        fNormal = vNormal;
        fLight = vLight;
        fAmbient = vAmbient;
        fTexCoord = vTexCoord;
}
