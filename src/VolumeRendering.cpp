#include "VolumeRendering.h"

namespace VolumeRendering {

void CalculatePipelineData(const Data::XRay &xRay, const Data::CT &ct, const vec<2> &sourceOffset, const vec<3> &pan, const mat<4, 4> &rotationMatrix, int samplesN, float range, float centre, SharedShaderData *sharedOut, VertexShaderData *vertexOut, FragmentShaderData *fragmentOut){
	
	const vec<3> sourcePosition = sourceOffset | xRay.sourceDistance;
	
	const mat<4, 4> C2 = {{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		(-1.0f * sourcePosition) | 1.0f
	}};
	
	const vec<3> ctDimensions = ct.pixelSpacing * static_cast<vec<3>>(ct.size);
	const mat<4, 4> C4 = mat<4, 4>::Scaling(ctDimensions) & mat<4, 4>::Translation({-0.5f, -0.5f, -0.5f});
	const mat<4, 4> F1 = C2 & mat<4, 4>::Translation(pan) & rotationMatrix & C4;
	
	sharedOut->B1 = F1.Inverted();
	
	const vec<2> xRayDimensions = xRay.pixelSpacing * static_cast<vec<2>>(xRay.size);
	const mat<4, 4> C5 = {{
		{1.0f / xRayDimensions.x, 0.0f, 0.0f, 0.0f},
		{0.0f, -1.0f / xRayDimensions.y, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.5f, 0.5f, 0.0f, 1.0f}
	}};
	const mat<4, 4> F2 = C5 & C2.Inverted();
	
	vertexOut->B2 = F2.Inverted();
	
	*fragmentOut = FragmentShaderData{
		.jumpSize = 2.0f * sqrtf(ctDimensions.SqMag()),
		.samplesN = samplesN,
		.nInv = 1.0f / float(samplesN),
		.range = range,
		.centre = centre
	};
}

} // namespace VolumeRendering
