#pragma once

#include "Data.h"

namespace VolumeRendering {

// probs want to separate B1, as this is shared between the shaders.
// probs want to not bother with B1_noTranslation, as this should be possible to extract within the fragment shader

struct VertexShaderData {
	mat<4, 4> B1;
	mat<4, 4> B2;
};

struct FragmentShaderData {
	mat<4, 4> B1;
	mat<3, 3> B1_noTranslation;
	float jumpSize;
	int samplesN;
	float nInv;
	float range;
	float centre;
};

struct PipelineData {
	VertexShaderData vertData;
	FragmentShaderData fragData;
};

PipelineData CalculatePipelineData(const Data::XRay &xRay, const vec<2> &sourceOffset, const vec<3> &pan, const mat<4, 4> rotationMatrix, int samplesN, float range, float centre);

} // namespace VolumeRendering
