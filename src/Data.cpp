#include <string>

#include "Data.h"

std::vector<std::string> SplitStringByBackslash(std::string str){
	static const std::string delimiter = "\\";
	size_t pos = 0;
	std::vector<std::string> ret {};
	while((pos = str.find(delimiter)) != std::string::npos){
		ret.push_back(str.substr(0, pos));
		str.erase(0, pos + delimiter.length());
	}
	ret.push_back(str);
	return ret;
}
std::vector<float> ParseStringToFloats(std::string str){
	const std::vector<std::string> subStrs = SplitStringByBackslash(str);
	std::vector<float> ret(subStrs.size());
	for(int i=0; i<subStrs.size(); ++i){
		ret[i] = std::stof(subStrs[i]);
	}
	return ret;
}

void ReadXRay(const char *filename){
	DcmFileFormat fileformat;
	OFCondition status = fileformat.loadFile(filename);
	if(status.good()){
		if(fileformat.getDataset()->tagExistsWithValue(DCM_PixelSpacing)){
			const char *pixelSpacingString;
			OFCondition readStatus = fileformat.getDataset()->findAndGetString(DCM_PixelSpacing, pixelSpacingString);
			if (readStatus.good()){
				std::vector<float> pixelSpacingFloats = ParseStringToFloats(std::string(pixelSpacingString));
				std::cout << "Pixel spacing = (" << pixelSpacingFloats[0] << ", " << pixelSpacingFloats[1] << ")" << std::endl;
			} else
				std::cerr << "Error: cannot access pixel spacing: " << readStatus.text() << std::endl;
		} else {
			std::cout << "Pixel spacing doesn't exist\n";
		}
	} else
		std::cerr << "Error: cannot read DICOM file (" << status.text() << ")" << std::endl;
	
	std::unique_ptr<DicomImage> image = std::make_unique<DicomImage>("test.dcm");
	if(image){
		if(image->getStatus() == EIS_Normal){
			if(image->isMonochrome()){
				image->setMinMaxWindow();
				const size_t dataSize = image->getOutputDataSize();
				if(!dataSize)
					std::cerr << "Error: cannot get DICOM image size" << std::endl;
				
				std::vector<uint8_t> pixels(dataSize);
				image->getOutputData(pixels.data(), dataSize, 8);
				//
			}
		} else
			std::cerr << "Error: cannot load DICOM image (" << DicomImage::getString(image->getStatus()) << ")" << std::endl;
	}
}
