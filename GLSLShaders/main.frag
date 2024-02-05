#version 450

layout(binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec2 v_texCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(vec3(texture(textureSampler, v_texCoord).r), 1.0);
}
