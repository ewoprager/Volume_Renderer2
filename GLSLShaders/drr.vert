#version 450

layout(binding = 0) uniform SharedUBO {
	mat4 B1;
} shared_ubo;

layout(binding = 1) uniform UBO {
	mat4 B2;
} ubo;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;

layout(location = 0) out vec2 v_texCoord;
layout(location = 1) out vec3 v_start_ctTextureSpace;
layout(location = 2) out vec3 v_start_SourceSpace;


void main() {
	v_start_ctTextureSpace = (shared_ubo.B1 * ubo.B2 * vec4(a_texCoord, 0.0, 1.0)).xyz;
	v_start_SourceSpace = (ubo.B2 * vec4(a_texCoord, 0.0, 1.0)).xyz;
	
	vec2 flippedTexCoord = a_texCoord;
	flippedTexCoord.y = 1.0 - flippedTexCoord.y;
	v_texCoord = flippedTexCoord;
	
	gl_Position = vec4(a_position.xy, 0.9, 1.0);
	
//	gl_Position = vec4(a_position, 0.0, 1.0);
//	v_texCoord = a_texCoord;
}
