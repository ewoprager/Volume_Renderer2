#version 450

layout(push_constant) uniform PCs {
	layout(offset = 28)
	float windowCentre;
	float windowWidth;
} pcs;

layout(binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec2 v_texCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(vec3((texture(textureSampler, v_texCoord).r - pcs.windowCentre + 0.5 * pcs.windowWidth) / pcs.windowWidth), 1.0);
}
