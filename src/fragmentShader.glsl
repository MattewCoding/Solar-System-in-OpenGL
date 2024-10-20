#version 330 core	     // Minimal GL version support expected from the GPU

uniform vec3 camPos;

in vec3 fPosition; // Position of the vertex
in vec3 fNormal; // Input from vertexShader = has to have same name as vertexShader's var
in vec3 fColor; // Input from vertexShader = has to have same name as vertexShader's var

out vec4 color; // Shader output: the color response attached to this fragment

void main() {
	vec3 n = normalize(fNormal);
	vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // hard-coded light

	vec3 v = normalize(camPos - fPosition);

	// Reflected light = mirrored across the normal AND going away from vertex, not towards it like light vector
	vec3 r = reflect(-l, n);

	vec3 ambient = vec3(0.0, 0.0, 0.0);
	vec3 diffuse = max(dot(n, l), 0.0) * vec3(1.0, 1.0, 1.0) * fColor;
	vec3 specular = pow(max(dot(v, r), 0.0), 32) * vec3(1.0, 1.0, 1.0) * fColor;

	color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB
}
