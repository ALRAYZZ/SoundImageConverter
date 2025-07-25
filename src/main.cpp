#include <iostream>
#include <sndfile.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <filesystem>




int main()
{
	// Check if wave file exists before trying to open it
	std::filesystem::path wavPath = "resources/sampleSound.wav";
	if (!std::filesystem::exists(wavPath))
	{
		std::cerr << "Error: WAV file does not exist at: " << std::filesystem::absolute(wavPath) << std::endl;
		std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
		return 1;
	}

	// Test libsndfile: Load a WAV file
	SF_INFO sfInfo;
	sfInfo.format = 0; // Initialize SF_INFO structure
	SNDFILE* audioFile = sf_open("resources/sampleSound.wav", SFM_READ, &sfInfo);
	if (!audioFile)
	{
		std::cerr << "Error: Could not open WAV file." << std::endl;
		std::cerr << "LibSndFile error: " << sf_strerror(audioFile) << std::endl;
		return 1;
	}
	std::cout << "WAV Info: " << sfInfo.frames << " samples, "
		<< sfInfo.channels << " channels, "
		<< sfInfo.samplerate << " Hz" << std::endl;
	sf_close(audioFile);


	// Test stb_image: Load a PNG file
	int width, height, channels;
	unsigned char* imageData = stbi_load("resources/sampleImage.png", &width, &height, &channels, 0);
	if (!imageData)
	{
		std::cerr << "Error: Could not open PNG file." << std::endl;
		return 1;
	}
	std::cout << "PNG Info: " << width << "x" << height << ", "
		<< channels << " channels" << std::endl;
	stbi_image_free(imageData); // Free the image data after use

	std::cout << "Library test successful!" << std::endl;
	return 0;
}