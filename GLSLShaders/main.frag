#version 450

layout(push_constant) uniform PCs {
	layout(offset = 28)
	float windowCentre;
	float windowWidth;
} pcs;

layout(binding = 0) uniform sampler2D textureSampler;
layout(binding = 1) uniform sampler2D drr;

layout(location = 0) in vec2 v_texCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(vec2((texture(textureSampler, v_texCoord).r - pcs.windowCentre + 0.5 * pcs.windowWidth) / pcs.windowWidth), texture(drr, v_texCoord).r, 1.0);
}
