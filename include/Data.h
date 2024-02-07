#pragma once

#include <vector>

#include <opencv4/opencv2/core/mat.hpp>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <vulkan/vulkan.h>
#include <mattresses.h>

#include "Header.h"

namespace Data {

class XRay;

namespace DICOM {

std::optional<XRay> LoadXRay(const char *filename);

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

} // namespace Data
