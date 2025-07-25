#include "SoundImageConverter/Converter.h"
#include <sndfile.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <vector>
#include <cstdint>
#include <cmath>
#include <iostream>

namespace SoundImageConverter
{
	bool Encoder::encode(const std::string& wavPath, const std::string& pngPath)
	{
		// Open WAV file
		SF_INFO sfInfo;
		sfInfo.format = 0;
		SNDFILE* audioFile = sf_open(wavPath.c_str(), SFM_READ, &sfInfo);
		if (!audioFile)
		{
			std::cerr << "Error: Could not open WAV file: " << wavPath << std::endl;
			return false;
		}

		// Determine bit depth and channels
		int channels = sfInfo.channels; // 1 = mono, 2 = stereo
		int sampleRate = sfInfo.samplerate; // Sample rate in Hz
		int bitDepth = (sfInfo.format & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_16 ? 16 : 8;
		sf_count_t numFrames = sfInfo.frames; // Total number of frames (samples per channel)

		// Calculate image dimensions
		const int width = 512; // Width of the image
		int channelsPerPixel = (bitDepth == 8) ? (channels == 1 ? 1 : 3) : 4;  // Grayscale for 8-bit mono, RGB for 8-bit stereo, RGBA for 16-bit
		int samplesNeeded = static_cast<int>(numFrames);
		int height = static_cast<int>(std::ceil(static_cast<double>(samplesNeeded) / width)) + 1; // +1 so that we have enough space for the metadata row

		std::cout << "Image dimensions: " << width << "x" << height << " with " << channelsPerPixel << " channels per pixel." << std::endl;


		// Read audio samples
		std::vector<int16_t> samples(numFrames * channels);
		sf_count_t readCount = sf_read_short(audioFile, samples.data(), numFrames * channels);
		sf_close(audioFile);

		if (readCount != numFrames * channels)
		{
			std::cerr << "Error: Failed to read all samples from WAV file. Expected: " << numFrames * channels << ", Read: " << readCount << std::endl;
			return false;
		}

		// Debug: Check first few samples
		std::cout << "First 10 samples: ";
		for (int i = 0; i < std::min(10, static_cast<int>(readCount)); i++)
		{
			std::cout << samples[i] << " ";
		}
		std::cout << std::endl;

		// Preapare image buffer
		// Creating the image buffer as big as the image dimensions plus taking into account the amount of bytes per pixel (channelsPerPixel)
		std::vector<uint8_t> image(width * height * channelsPerPixel, 0);

		// Embed metada in first row
		uint32_t sampleRate32 = static_cast<uint32_t>(sampleRate);
		image[0] = (sampleRate32 >> 24) & 0xFF; // Sample rate: byte 1
		image[1] = (sampleRate32 >> 16) & 0xFF; // Sample rate: byte 2
		image[2] = (sampleRate32 >> 8) & 0xFF;  // Sample rate: byte 3
		image[3] = sampleRate32 & 0xFF;         // Sample rate: byte 4
		image[4] = static_cast<uint8_t>(channels); // Channels
		image[5] = static_cast<uint8_t>(bitDepth); // Bit depth

		// Helper function to convert 16-bit s ample to 8-bit pixel value
		auto sampleToPixel = [](int16_t sample) -> uint8_t
		{
			return static_cast<uint8_t>((sample + 32768) / 256);
		};


		// Encode samples to pixels
		size_t pixelIndex = width * channelsPerPixel; // Start after metadata row
		size_t samplesProcessed = 0;

		for (sf_count_t i = 0; i < numFrames && pixelIndex < image.size(); i++)
		{
			if (channelsPerPixel == 1) // Mono Grayscale
			{
				if (pixelIndex < image.size()) // Mono 8-bit
				{
					image[pixelIndex++] = sampleToPixel(samples[i]);
					samplesProcessed++;
				}
			}
			else if (channelsPerPixel == 3) // Stereo RGB
			{
				if (pixelIndex + 2 < image.size())
				{
					image[pixelIndex++] = sampleToPixel(samples[i * 2]);     // Left channel (Red)
					image[pixelIndex++] = sampleToPixel(samples[i * 2 + 1]); // Right channel (Green)
					image[pixelIndex++] = 128; // Blue channel (constant value for variation)
					samplesProcessed++;
				}
			}
			else if (channelsPerPixel == 4) // 16-bit RGBA
			{
				if (pixelIndex + 3 < image.size())
				{
					int16_t left = (channels == 1) ? samples[i] : samples[i * 2];
					image[pixelIndex++] = sampleToPixel(left); // Red channel
					image[pixelIndex++] = sampleToPixel(left >> 1); // Green channel

					if (channels == 2)
					{
						int16_t right = samples[i * 2 + 1];
						image[pixelIndex++] = sampleToPixel(right); // Blue channel
						image[pixelIndex++] = 255; // Alpha channel (fully opaque)
					}
					else
					{
						image[pixelIndex++] = sampleToPixel(left >> 2); // Blue channel
						image[pixelIndex++] = 255; // Alpha channel (fully opaque)
					}
					samplesProcessed++;
				}
			}
		}

		std::cout << "Processed " << samplesProcessed << " samples into " << pixelIndex << " bytes " << std::endl;

		// Debug: Check first few pixels
		std::cout << "First 10 pixels after metadata: ";
		size_t metadataEnd = width * channelsPerPixel;
		for (int i = 0; i < std::min(10, static_cast<int>(image.size() - metadataEnd)); i++)
		{
			std::cout << static_cast<int>(image[metadataEnd + i]) << " ";
		}

		// Write PNG
		int result = stbi_write_png(pngPath.c_str(), width, height, channelsPerPixel, image.data(), width * channelsPerPixel);
		if (!result)
		{
			std::cerr << "Error: Failed to write PNG file: " << pngPath << std::endl;
			return false;
		}

		std::cout << "Encoded " << wavPath << " to " << pngPath << std::endl;
		return true;
	}
} // namespace SoundImageConverter