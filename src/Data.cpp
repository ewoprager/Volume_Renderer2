#include <string>
#include <filesystem>
#include <set>

#include <evk/Base.hpp>

#include "Data.h"

namespace Data {

namespace DICOM {

void PrintType(DcmDataset *dataset, const DcmTagKey &key);

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
	
	// image data
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

std::optional<CT> LoadCT(const char *pathname){
	
	const std::filesystem::path directory = pathname;
	if(!std::filesystem::is_directory(directory)){
		return {};
	}
	
	std::set<std::filesystem::path> sortedFiles {};
	for(const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(directory)){
		sortedFiles.insert(entry.path());
	}
	
	if(sortedFiles.size() < 3){
		// ToDo: Make it wxWidgets message box
		std::cout << "Fewer than 3 files in CT directory.\n";
		return {};
	}
	
	CT ret;
	ret.size.z = sortedFiles.size();
	vec<3> imagePositionPatient;
	vec<6> imageOrientationPatient;
	bool orderReversed;
	size_t sliceSize;
	
	std::set<std::filesystem::path>::const_iterator it = sortedFiles.begin();
	{ // first
		DcmFileFormat fileformat;
		OFCondition status = fileformat.loadFile(it->string());
		if(!status.good()){
			std::cerr << "Error: cannot read DICOM file '" << it->string() << "': " << status.text() << "\n";
			return {};
		}
		{ // pixel spacing
			std::optional<std::vector<float>> pixelSpacing = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_PixelSpacing, 2);
			if(!pixelSpacing){
				std::cerr << "Error: cannot read pixel spacing from file " << it->string() << "\n";
				return {};
			}
			ret.pixelSpacing.xy_r() = {pixelSpacing.value()[1], pixelSpacing.value()[0]}; // flipping to x,y rather than y,x
		}
		{ // image position patient
			std::optional<std::vector<float>> ipp = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_ImagePositionPatient, 3);
			if(!ipp){
				std::cerr << "Error: cannot imagePositionPatient spacing from file " << it->string() << "\n";
				return {};
			}
			imagePositionPatient = *reinterpret_cast<vec<3> *>(ipp->data());
		}
		{ // image orientation patient
			std::optional<std::vector<float>> iop = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_ImageOrientationPatient, 6);
			if(!iop){
				std::cerr << "Error: cannot read imageOrientationPatient from file " << it->string() << "\n";
				return {};
			}
			imageOrientationPatient = *reinterpret_cast<vec<6> *>(iop->data());
		}
		{ // rows
			std::optional<uint16_t> rows = ReadInt_Uint16(fileformat.getDataset(), DCM_Rows);
			if(!rows){
				std::cerr << "Error: cannot read no. of rows from file " << it->string() << "\n";
				return {};
			}
			ret.size.y = rows.value();
		}
		{ // columns
			std::optional<uint16_t> cols = ReadInt_Uint16(fileformat.getDataset(), DCM_Columns);
			if(!cols){
				std::cerr << "Error: cannot read no. of cols from file " << it->string() << "\n";
				return {};
			}
			ret.size.x = cols.value();
		}
		{ // image data
			std::unique_ptr<DicomImage> image = std::make_unique<DicomImage>(it->string().data());
			if(!image){
				std::cerr << "Error: image from file " << it->string() << " is loaded as NULL\n";
				return {};
			}
			if(image->getStatus() != EIS_Normal){
				std::cerr << "Error: cannot load DICOM image from file " << it->string() << ": " << DicomImage::getString(image->getStatus()) << "\n";
				return {};
			}
			if(!image->isMonochrome()){
				std::cerr << "Error: X-Ray DICOM image loaded from " << it->string() << " is not monochrome\n";
				return {};
			}
			image->setMinMaxWindow();
			sliceSize = image->getOutputDataSize();
			const size_t N = ret.size.x * ret.size.y;
			if(!sliceSize || sliceSize % N){
				std::cerr << "Error: DICOM image size from " << it->string() << " is inconsistent with Rows * Columns\n";
				return {};
			}
			ret.imageDepth = sliceSize / N;
			ret.data = std::vector<uint8_t>(sliceSize * (sortedFiles.size() + 1));
			if(!image->getOutputData(ret.data.data(), sliceSize, 8 * int(ret.imageDepth))){
				std::cerr << "Error: cannot read DICOM image data from " << it->string() << "\n";
				return {};
			}
		}
	}
	
	++it;
	{ // second
		DcmFileFormat fileformat;
		OFCondition status = fileformat.loadFile(it->string());
		if(!status.good()){
			std::cerr << "Error: cannot read DICOM file '" << it->string() << "': " << status.text() << "\n";
			return {};
		}
		{ // pixel spacing
			std::optional<std::vector<float>> pixelSpacing = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_PixelSpacing, 2);
			if(!pixelSpacing){
				std::cerr << "Error: cannot read pixel spacing from file " << it->string() << "\n";
				return {};
			}
			assert(ret.pixelSpacing.xy() == vec<2>({pixelSpacing.value()[1], pixelSpacing.value()[0]})); // flipping to x,y rather than y,x
		}
		{ // images position patient
			std::optional<std::vector<float>> ipp = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_ImagePositionPatient, 3);
			if(!ipp){
				std::cerr << "Error: cannot imagePositionPatient spacing from file " << it->string() << "\n";
				return {};
			}
			const vec<3> _ipp = *reinterpret_cast<vec<3> *>(ipp->data());
			assert(_ipp.xy() == imagePositionPatient.xy());
			ret.pixelSpacing.z = _ipp.z - imagePositionPatient.z;
			orderReversed = ret.pixelSpacing.z < 0.0f;
			if(orderReversed) ret.pixelSpacing.z = -ret.pixelSpacing.z;
		}
		{ // rows
			std::optional<uint16_t> rows = ReadInt_Uint16(fileformat.getDataset(), DCM_Rows);
			if(!rows){
				std::cerr << "Error: cannot read no. of rows from file " << it->string() << "\n";
				return {};
			}
			assert(ret.size.y == rows.value());
		}
		{ // columns
			std::optional<uint16_t> cols = ReadInt_Uint16(fileformat.getDataset(), DCM_Columns);
			if(!cols){
				std::cerr << "Error: cannot read no. of cols from file " << it->string() << "\n";
				return {};
			}
			assert(ret.size.x == cols.value());
		}
		{ // image data
			std::unique_ptr<DicomImage> image = std::make_unique<DicomImage>(it->string().data());
			if(!image){
				std::cerr << "Error: image from file " << it->string() << " is loaded as NULL\n";
				return {};
			}
			if(image->getStatus() != EIS_Normal){
				std::cerr << "Error: cannot load DICOM image from file " << it->string() << ": " << DicomImage::getString(image->getStatus()) << "\n";
				return {};
			}
			if(!image->isMonochrome()){
				std::cerr << "Error: X-Ray DICOM image loaded from " << it->string() << " is not monochrome\n";
				return {};
			}
			image->setMinMaxWindow();
			assert(image->getOutputDataSize() == sliceSize);
			if(!image->getOutputData(ret.data.data() + sliceSize, sliceSize, 8 * int(ret.imageDepth))){
				std::cerr << "Error: cannot read DICOM image data from " << it->string() << "\n";
				return {};
			}
		}
	}
	
	const vec<3> X0 = *reinterpret_cast<vec<3> *>(&imageOrientationPatient);
	const vec<3> Y0 = *reinterpret_cast<vec<3> *>(&imageOrientationPatient[3]);
	const vec<3> spaceDimensions = (static_cast<vec<2>>(ret.size.xy()) * ret.pixelSpacing.xy()) | (static_cast<float>(sortedFiles.size()) * ret.pixelSpacing.z);
	vec<3> dataOriginOffset = imagePositionPatient + 0.5f * spaceDimensions;
	if(orderReversed) dataOriginOffset[2] -= spaceDimensions[2];
	size_t i = 2;
	
	ret.C1Inverse = mat<4, 4>{{
		X0 | 0.0f,
		Y0 | 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		dataOriginOffset | 1.0f
	}}.Inverted();
	
	std::cout << ret.C1Inverse << "\n";
	
	for(; it != sortedFiles.end(); ++it){
		DcmFileFormat fileformat;
		OFCondition status = fileformat.loadFile(it->string());
		if(!status.good()){
			std::cerr << "Error: cannot read DICOM file '" << it->string() << "': " << status.text() << "\n";
			return {};
		}
		{ // rows
			std::optional<uint16_t> rows = ReadInt_Uint16(fileformat.getDataset(), DCM_Rows);
			if(!rows){
				std::cerr << "Error: cannot read no. of rows from file " << it->string() << "\n";
				return {};
			}
			assert(rows.value() == ret.size.y);
		}
		{ // columns
			std::optional<uint16_t> cols = ReadInt_Uint16(fileformat.getDataset(), DCM_Columns);
			if(!cols){
				std::cerr << "Error: cannot read no. of cols from file " << it->string() << "\n";
				return {};
			}
			assert(cols.value() == ret.size.x);
		}
		{ // pixel spacing
			std::optional<std::vector<float>> pixelSpacing = ReadFloats_OFStringArray(fileformat.getDataset(), DCM_PixelSpacing, 2);
			if(!pixelSpacing){
				std::cerr << "Error: cannot read pixel spacing from file " << it->string() << "\n";
				return {};
			}
			assert(ret.pixelSpacing.xy() == vec<2>({pixelSpacing.value()[1], pixelSpacing.value()[0]})); // flipping to x,y rather than y,x
		}
		{ // image data
			std::unique_ptr<DicomImage> image = std::make_unique<DicomImage>(it->string().data());
			if(!image){
				std::cerr << "Error: image from file " << it->string() << " is loaded as NULL\n";
				return {};
			}
			if(image->getStatus() != EIS_Normal){
				std::cerr << "Error: cannot load DICOM image from file " << it->string() << ": " << DicomImage::getString(image->getStatus()) << "\n";
				return {};
			}
			if(!image->isMonochrome()){
				std::cerr << "Error: X-Ray DICOM image loaded from " << it->string() << " is not monochrome\n";
				return {};
			}
			image->setMinMaxWindow();
			assert(image->getOutputDataSize() == sliceSize);
			if(!image->getOutputData(ret.data.data() + sliceSize * i, sliceSize, 8 * int(ret.imageDepth))){
				std::cerr << "Error: cannot read DICOM image data from " << it->string() << "\n";
				return {};
			}
		}
		++i;
	}
	
	return ret;
}


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

} // namespace DICOM

VkFormat MonochromeVKFormatFromImageDepth(size_t imageDepth){
	switch(imageDepth){
		case 1:
			return VK_FORMAT_R8_UNORM;
		case 2:
			return VK_FORMAT_R16_UNORM;
		case 4:
			return VK_FORMAT_R32_SFLOAT;
		default:
			throw std::runtime_error("Unhandled image depth; no known format");
	}
}

int MonochromeCVFormatFromImageDepth(size_t imageDepth){
	switch(imageDepth){
		case 1:
			return CV_8U;
		case 2:
			return CV_16U;
		case 4:
			return CV_32F;
		default:
			throw std::runtime_error("Unhandled image depth; no known format");
	}
}

} // namespace Data
