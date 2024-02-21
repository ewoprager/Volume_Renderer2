#pragma once

#include "Data.h"

namespace VolumeRendering {

struct SharedShaderData {
	mat<4, 4> B1;
};

struct VertexShaderData {
	mat<4, 4> B2;
};

struct FragmentShaderData {
	float jumpSize;
	int samplesN;
	float nInv;
	float range;
	float centre;
};

void CalculatePipelineData(const Data::XRay &xRay, const Data::CT &ct, const vec<2> &sourceOffset, const vec<3> &pan, const mat<4, 4> &rotationMatrix, int samplesN, float range, float centre, SharedShaderData *sharedOut, VertexShaderData *vertexOut, FragmentShaderData *fragmentOut);

} // namespace VolumeRendering
