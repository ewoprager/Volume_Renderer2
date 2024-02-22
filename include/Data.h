#pragma once

#include <vector>

#include <opencv4/opencv2/core/mat.hpp>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <vulkan/vulkan.h>

#include "Header.h"

namespace Data {

struct params_t : public vec<8> {
	vec<2> &SourceOffset(){ return *reinterpret_cast<vec<2> *>(&vec<8>::operator[](0)); }
	vec<3> &Pan(){ return *reinterpret_cast<vec<3> *>(&vec<8>::operator[](2)); }
	vec<3> &Rotation(){ return *reinterpret_cast<vec<3> *>(&vec<8>::operator[](5)); }
	const vec<2> &SourceOffset() const { return *reinterpret_cast<const vec<2> *>(&vec<8>::operator[](0)); }
	const vec<3> &Pan() const { return *reinterpret_cast<const vec<3> *>(&vec<8>::operator[](2)); }
	const vec<3> &Rotation() const { return *reinterpret_cast<const vec<3> *>(&vec<8>::operator[](5)); }
	
	friend std::ostream &operator<<(std::ostream &stream, const params_t &val){
		return stream << "Source offset: " << val.SourceOffset() << ", Pan: " << val.Pan() << ", Rotation: " << val.Rotation();
	}
};

struct XRay;
struct CT;

inline mat<4, 4> ToRotationMatrix(const vec<3> &rotation){
	return mat<4, 4>::ZRotation(rotation.x) & mat<4, 4>::XRotation(rotation.y) & mat<4, 4>::YRotation(rotation.z) & mat<4, 4>::XRotation(-0.5f * float(M_PI));
}

namespace DICOM {

std::optional<XRay> LoadXRay(const char *filename);
std::optional<CT> LoadCT(const char *filename);

} // namespace DICOM

VkFormat MonochromeVKFormatFromImageDepth(size_t imageDepth);
int MonochromeCVFormatFromImageDepth(size_t imageDepth);

struct XRay {
	vec<2, uint16_t> size;
	vec<2> pixelSpacing;
	float sourceDistance;
	std::vector<uint8_t> data;
	size_t imageDepth;
	
	cv::Mat ToCVMatrix(){
		return cv::Mat(size.y, size.x, MonochromeCVFormatFromImageDepth(imageDepth), data.data());
	}
};

struct CT {
	vec<3, uint16_t> size;
	vec<3> pixelSpacing;
	mat<4, 4> C1Inverse;
	bool leftImplantPresent;
	bool rightImplantPresent;
	std::vector<uint8_t> data;
	size_t imageDepth;
};

} // namespace Data
