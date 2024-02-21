#version 450

layout(push_constant) uniform PCs {
	layout(offset = 28)
	float windowCentre;
	float windowWidth;
	bool haveXRay;
} pcs;

layout(set = 0, binding = 0) uniform texture2D xray;
layout(set = 1, binding = 0) uniform texture2D drr;
layout(set = 1, binding = 1) uniform sampler _sampler;

layout(location = 0) in vec2 v_texCoord;

layout(location = 0) out vec4 outColor;

void main() {
	float drrValue = texture(sampler2D(drr, _sampler), v_texCoord).r;
	if(pcs.haveXRay){
		outColor = vec4(vec2((texture(sampler2D(xray, _sampler), v_texCoord).r - pcs.windowCentre + 0.5 * pcs.windowWidth) / pcs.windowWidth), drrValue, 1.0);
	} else {
		outColor = vec4(vec3(drrValue), 1.0);
	}
}
