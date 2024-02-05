#include <string>

#include <evk/Base.hpp>

#include "Data.h"

namespace Data {

namespace DICOM {

void PrintType(DcmDataset *dataset, const DcmTagKey &key){
	{  const char *		value;  OFCondition readStatus = dataset->findAndGetString			(key, value); if(readStatus.good()){ std::cout << "Type const char *	 worked: " << value << "\n"; }  }
	{  OFString			value;  OFCondition readStatus = dataset->findAndGetOFString 		(key, value); if(readStatus.good()){ std::cout << "Type OFString		 worked: " << value << "\n"; }  }
	{  OFString 		value;  OFCondition readStatus = dataset->findAndGetOFStringArray 	(key, value); if(readStatus.good()){ std::cout << "Type OFString array	 worked: " << value << "\n"; }  }
	{  Uint8 			value;  OFCondition readStatus = dataset->findAndGetUint8 			(key, value); if(readStatus.good()){ std::cout << "Type Uint8 			 worked: " << value << "\n"; }  }
	{  const Uint8 * 	value;  OFCondition readStatus = dataset->findAndGetUint8Array 		(key, value); if(readStatus.good()){ std::cout << "Type const Uint8 * 	 worked: " << value << "\n"; }  }
	{  Uint16 			value;  OFCondition readStatus = dataset->findAndGetUint16 			(key, value); if(readStatus.good()){ std::cout << "Type Uint16 			 worked: " << value << "\n"; }  }
	{  const Uint16 *	value;  OFCondition readStatus = dataset->findAndGetUint16Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Uint16 *	 worked: " << value << "\n"; }  }
	{  Sint16 			value;  OFCondition readStatus = dataset->findAndGetSint16 			(key, value); if(readStatus.good()){ std::cout << "Type Sint16 			 worked: " << value << "\n"; }  }
	{  const Sint16 *	value;  OFCondition readStatus = dataset->findAndGetSint16Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Sint16 *	 worked: " << value << "\n"; }  }
	{  Uint32 			value;  OFCondition readStatus = dataset->findAndGetUint32 			(key, value); if(readStatus.good()){ std::cout << "Type Uint32 			 worked: " << value << "\n"; }  }
	{  const Uint32 * 	value;  OFCondition readStatus = dataset->findAndGetUint32Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Uint32 * 	 worked: " << value << "\n"; }  }
	{  Sint32 			value;  OFCondition readStatus = dataset->findAndGetSint32 			(key, value); if(readStatus.good()){ std::cout << "Type Sint32 			 worked: " << value << "\n"; }  }
	{  const Sint32 * 	value;  OFCondition readStatus = dataset->findAndGetSint32Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Sint32 * 	 worked: " << value << "\n"; }  }
	{  Uint64 			value;  OFCondition readStatus = dataset->findAndGetUint64 			(key, value); if(readStatus.good()){ std::cout << "Type Uint64 			 worked: " << value << "\n"; }  }
	{  const Uint64 * 	value;  OFCondition readStatus = dataset->findAndGetUint64Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Uint64 * 	 worked: " << value << "\n"; }  }
	{  Sint64 			value;  OFCondition readStatus = dataset->findAndGetSint64 			(key, value); if(readStatus.good()){ std::cout << "Type Sint64 			 worked: " << value << "\n"; }  }
	{  const Sint64 * 	value;  OFCondition readStatus = dataset->findAndGetSint64Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Sint64 * 	 worked: " << value << "\n"; }  }
	{  long int 		value;  OFCondition readStatus = dataset->findAndGetLongInt 		(key, value); if(readStatus.good()){ std::cout << "Type long int 		 worked: " << value << "\n"; }  }
	{  Float32 			value;  OFCondition readStatus = dataset->findAndGetFloat32 		(key, value); if(readStatus.good()){ std::cout << "Type Float32 		 worked: " << value << "\n"; }  }
	{  const Float32 * 	value;  OFCondition readStatus = dataset->findAndGetFloat32Array 	(key, value); if(readStatus.good()){ std::cout << "Type const Float32 *  worked: " << value << "\n"; }  }
	{  Float64 			value;  OFCondition readStatus = dataset->findAndGetFloat64 		(key, value); if(readStatus.good()){ std::cout << "Type Float64 		 worked: " << value << "\n"; }  }
	{  const Float64 * 	value;  OFCondition readStatus = dataset->findAndGetFloat64Array	(key, value); if(readStatus.good()){ std::cout << "Type const Float64 *  worked: " << value << "\n"; }  }
}

std::vector<std::string> SplitString(std::string str){
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
	const std::vector<std::string> subStrs = SplitString(str);
	std::vector<float> ret(subStrs.size());
	for(int i=0; i<subStrs.size(); ++i){
		ret[i] = std::stof(subStrs[i]);
	}
	return ret;
}
std::optional<std::vector<float>> ReadFloats_OFStringArray(DcmDataset *dataset, const DcmTagKey &key, size_t expectedLength=0){
	if(!dataset->tagExistsWithValue(key))
		return {};
	
	std::string string;
	OFCondition readStatus = dataset->findAndGetOFStringArray(key, string);
	if(!readStatus.good())
		return {};
	
	std::vector<float> ret = ParseStringToFloats(string);
	
	if(expectedLength){
		if(ret.size() != expectedLength)
			return {};
	}
	
	return ret;
}
std::optional<Float64> ReadFloat_Float64(DcmDataset *dataset, const DcmTagKey &key){
	if(!dataset->tagExistsWithValue(key))
		return {};
	
	Float64 value;
	OFCondition readStatus = dataset->findAndGetFloat64(key, value);
	if(!readStatus.good())
		return {};
	
	return value;
}
std::optional<uint16_t> ReadInt_Uint16(DcmDataset *dataset, const DcmTagKey &key){
	if(!dataset->tagExistsWithValue(key))
		return {};
	
	uint16_t value;
	OFCondition readStatus = dataset->findAndGetUint16(key, value);
	if(!readStatus.good())
		return {};
	
	return value;
}

std::optional<XRay> LoadXRay(const char *filename){
	DcmFileFormat fileformat;
	OFCondition status = fileformat.loadFile(filename);
	if(!status.good()){
		std::cerr << "Error: cannot read DICOM file '" << filename << "': " << status.text() << "\n";
		return {};
	}
	
	XRay ret;
	
	{ // pixel spacing and source depth
		std::optional<std::vector<float>> pixelSpacing = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_PixelSpacing, 2);
		if(pixelSpacing){
			// 'PixelSpacing' works with 'DistanceSourceToPatient'
			std::optional<Float64> distanceSourceToPatient = ReadFloat_Float64(fileformat.getDataset(), DCM_DistanceSourceToPatient);
			if(distanceSourceToPatient){
				ret.pixelSpacing = {pixelSpacing.value()[1], pixelSpacing.value()[0]}; // flipping to x,y rather than y,x
				ret.sourceDistance = distanceSourceToPatient.value();
			} else
				pixelSpacing = {};
		}
		if(!pixelSpacing){
			// otherwise we need 'ImagerPixelSpacing' and 'DistanceSourceToDetector', which also work together
			std::optional<std::vector<float>> imagerPixelSpacing = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_ImagerPixelSpacing, 2);
			if(!imagerPixelSpacing){
				std::cerr << "Error: cannot read valid pixel spacing <-> source depth pair from file " << filename << "\n";
				return {};
			}
			std::optional<Float64> distanceSourceToDetector = ReadFloat_Float64(fileformat.getDataset(), DCM_DistanceSourceToDetector);
			if(!distanceSourceToDetector){
				std::cerr << "Error: cannot read valid pixel spacing <-> source depth pair from file " << filename << "\n";
				return {};
			}
			ret.pixelSpacing = {imagerPixelSpacing.value()[1], imagerPixelSpacing.value()[0]}; // flipping to x,y rather than y,x
			ret.sourceDistance = distanceSourceToDetector.value();
		}
	}
	
	{ // rows
		std::optional<uint16_t> rows = ReadInt_Uint16(fileformat.getDataset(), DCM_Rows);
		if(!rows){
			std::cerr << "Error: cannot read no. of rows from file " << filename << "\n";
			return {};
		}
		ret.size.y = rows.value();
	}
	
	{ // columns
		std::optional<uint16_t> cols = ReadInt_Uint16(fileformat.getDataset(), DCM_Columns);
		if(!cols){
			std::cerr << "Error: cannot read no. of cols from file " << filename << "\n";
			return {};
		}
		ret.size.x = cols.value();
	}
	
	std::unique_ptr<DicomImage> image = std::make_unique<DicomImage>(filename);
	if(!image){
		std::cerr << "Error: image from file " << filename << " is loaded as NULL\n";
		return {};
	}
	if(image->getStatus() != EIS_Normal){
		std::cerr << "Error: cannot load DICOM image from file " << filename << ": " << DicomImage::getString(image->getStatus()) << "\n";
		return {};
	}
	if(!image->isMonochrome()){
		std::cerr << "Error: X-Ray DICOM image loaded from " << filename << " is not monochrome\n";
		return {};
	}
	image->setMinMaxWindow();
	const size_t imageSize = image->getOutputDataSize();
	const size_t N = ret.size.x * ret.size.y;
	if(!imageSize || imageSize % N){
		std::cerr << "Error: DICOM image size from " << filename << " is inconsistent with Rows * Columns\n";
		return {};
	}
	ret.imageDepth = imageSize / N;
	ret.data = std::vector<uint8_t>(imageSize);
	if(!image->getOutputData(ret.data.data(), imageSize, 8 * int(ret.imageDepth))){
		std::cerr << "Error: cannot read DICOM image data from " << filename << "\n";
		return {};
	}
	return ret;
}

VkFormat MonochromeFormatFromImageDepth(size_t imageDepth){
	switch(imageDepth){
		case 1:
			return VK_FORMAT_R8_UNORM;
		case 2:
			return VK_FORMAT_R16_UNORM;
		case 4:
			return VK_FORMAT_R32_UINT;
		default:
			throw std::runtime_error("Unhandled image depth; no known format");
	}
}

} // namespace DICOM

} // namespace Data
