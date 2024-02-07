#include <opencv4/opencv2/imgproc.hpp>

#include "ImProc.h"

namespace Data {

void GaussianBlur(XRay &xray){
	cv::Mat matrix = xray.ToCVMatrix();
	const double sigma = 1.78;
	const int kernalSize =  static_cast<int>(std::floor(3.0 * sigma)) * 2 + 1;
	cv::GaussianBlur(matrix, matrix, {kernalSize, kernalSize}, sigma);
}

} // namespace Data
