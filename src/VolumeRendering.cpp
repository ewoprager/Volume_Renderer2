#include "VolumeRendering.h"

namespace VolumeRendering {

PipelineData CalculatePipelineData(const Data::XRay &xRay, const vec<2> &sourceOffset, const vec<3> &pan, const mat<4, 4> rotationMatrix, int samplesN, float range, float centre,
								   // will change this to be in CT:
const mat<4, 4> &C4
								   //
								   ){
	
	PipelineData ret;
	
	ret.fragData.nInv = 1.0f / float(samplesN);
	ret.fragData.range = range;
	ret.fragData.samplesN = samplesN;
	
	const vec<3> sourcePosition = sourceOffset | xRay.sourceDistance;
	
	const mat<4, 4> C2 = {{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		(-1.0f * sourcePosition) | 1.0f
	}};
	
	const mat<4, 4> F1 = C2 & mat<4, 4>::Translation(pan) & rotationMatrix & C4;
	ret.fragData.B1 = ret.vertData.B1 = F1.Inverted();
	
	ret.fragData.B1_noTranslation = {{
		ret.fragData.B1[0].xyz(),
		ret.fragData.B1[1].xyz(),
		ret.fragData.B1[2].xyz()
	}};
	
	const vec<2> xRayDimensions = xRay.pixelSpacing * static_cast<vec<2>>(xRay.size);
	
	const mat<4, 4> C5 = {{
		{1.0f / xRayDimensions.x, 0.0f, 0.0f, 0.0f},
		{0.0f, -1.0f / xRayDimensions.y, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.5f, 0.5f, 0.0f, 1.0f}
	}};
	const mat<4, 4> F2 = C5 & C2.Inverted();
	ret.vertData.B2 = F2.Inverted();
}

} // namespace VolumeRendering
