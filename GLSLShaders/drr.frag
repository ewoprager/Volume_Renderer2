#version 450

layout(binding = 0) uniform SharedUBO {
	mat4 B1;
} shared_ubo;

layout(binding = 2) uniform UBO {
	float jumpSize;
	int samplesN;
	float nInv;
	float range;
	float centre;
} ubo;

layout(location = 0) in vec2 v_texCoord;
layout(location = 1) in vec3 v_start_ctTextureSpace;
layout(location = 2) in vec3 v_start_SourceSpace;

layout(binding = 3) uniform sampler3D ct;

layout(location = 0) out vec4 outColor;

void main() {
	
	vec3 rayJump = mat3(shared_ubo.B1) * ubo.jumpSize * normalize(v_start_SourceSpace);
	
	vec4 sum = vec4(0.0);
	//for(int i=-u_samplesN/2; i<u_samplesN/2; i++) sum += vec<4>(texture3D(u_data, v_start_ctTextureSpace + float(i)*rayJump).r);
	for(int i=-ubo.samplesN/2; i<ubo.samplesN/2; i++){
		vec3 vec = v_start_ctTextureSpace + float(i) * rayJump;
		sum += vec4(texture(ct, vec3(vec.y, vec.x, vec.z)).r);
	}
	
	// 1.0 - (exp(-(sum.r - 0.296997)*u_nInv/8.09974) - u_centre + u_range*0.5)/u_range;
	// ((sum.r - 0.296997)*u_nInv/8.09974 - u_centre + u_range*0.5)/u_range;
	
	float v = 1.0 - (exp(-sum.r * ubo.nInv) - ubo.centre + ubo.range*0.5)/ubo.range;
	
	outColor = vec4(vec3(v), 1.0);
	
//	outColor = vec4(vec3(texture(ct, vec3(v_texCoord, 0.0)).r), 1.0);
}
