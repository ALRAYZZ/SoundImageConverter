#include <iostream>
#include "SoundImageConverter/Converter.h"
#include <filesystem>
#include <sstream>

std::string generateUniqueFileName(const std::string& basePath)
{
	if (!std::filesystem::exists(basePath))
	{
		return basePath;
	}

	std::filesystem::path path(basePath);
	std::string stem = path.stem().string();
	std::string extension = path.extension().string();
	std::string directory = path.parent_path().string();

	int counter = 1;
	std::string newPath;
	do
	{
		std::ostringstream oss;
		oss << directory << "/" << stem << "_" << counter << extension;
		newPath = oss.str();
		counter++;
	} while (std::filesystem::exists(newPath));

	return newPath;
	
}


int main()
{
	std::string wavPath = "resources/sampleSound.wav";
	std::string pngPath = generateUniqueFileName("resources/sampleSound.png");
	std::string wavOutPath = generateUniqueFileName("resources/sampleSound_decoded.wav");


	// Test encoding
	if (SoundImageConverter::Encoder::encode(wavPath, pngPath))
	{
		std::cout << "Encoding successful!" << std::endl;
	}
	else
	{
		std::cerr << "Encoding failed!" << std::endl;
		return 1;
	}

	// Test decoding
	if (SoundImageConverter::Decoder::decode(pngPath, wavOutPath))
	{
		std::cout << "Decoding successful!" << std::endl;
	}
	else
	{
		std::cerr << "Decoding failed!" << std::endl;
		return 1;
	}

	return 0;
}