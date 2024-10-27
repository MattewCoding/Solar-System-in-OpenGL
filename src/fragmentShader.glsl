#version 330 core	     // Minimal GL version support expected from the GPU

struct Material {
	sampler2D albedoTex; // texture unit, relate to glActivateTexture(GL_TEXTURE0 + i)
};

uniform vec3 camPos;
uniform Material material;

in vec3 fPosition; // Position of the vertex
in vec3 fNormal; // Input from vertexShader = has to have same name as vertexShader's out var
in vec3 fLight;
in vec3 fAmbient;
in vec2 fTexCoord;

out vec4 color; // Shader output: the color response attached to this fragment

void main() {
	//////    Texture stuff    //////
	vec3 texColor = texture(material.albedoTex, fTexCoord).rgb; // Sample texture color

	//////     Light stuff     //////
	vec3 n = normalize(fNormal);

	// ref: left-right, bottom-top, back-front
	// right hand
	vec3 l = fLight;
	//vec3 l = normalize(vec3(0.,0.,1.));

	vec3 v = normalize(camPos - fPosition);

	// Reflected light = mirrored across the normal AND going away from vertex, not towards it like light vector
	vec3 r = reflect(-l, n);

	vec3 ambient = fAmbient;
	vec3 diffuse = max(dot(n, l), 0.0) * vec3(1.0, 1.0, 1.0) * texColor;
	vec3 specular = pow(max(dot(v, r), 0.0), 8) * vec3(1.0, 1.0, 1.0) * texColor;

	color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB

}
