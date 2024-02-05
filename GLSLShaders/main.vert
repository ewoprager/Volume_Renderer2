#version 450

layout(push_constant) uniform PCs {
	vec2 viewPos;
	vec2 xraySize;
	vec2 viewSize;
	float zoom;
} pcs;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;

layout(location = 0) out vec2 v_texCoord;

void main() {
	gl_Position = vec4((a_position * pcs.xraySize * 0.5 - pcs.viewPos) * pcs.zoom / (pcs.viewSize * 0.5), 0.0, 1.0);
	v_texCoord = a_texCoord;
}
