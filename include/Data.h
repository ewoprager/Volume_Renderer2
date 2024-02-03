#pragma once

#include <vector>

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <mattresses.h>

#include "Header.h"

namespace Data {

namespace DICOM {

class XRay;

std::optional<XRay> LoadXRay(const char *filename);

struct XRay {
	vec<2, uint16_t> size;
	vec<2> pixelSpacing;
	float sourceDistance;
	std::vector<uint8_t> data;
	size_t imageDepth;
};

} // namespace DICOM

} // namespace Data
