#include "SoundImageConverter/Converter.h"
#include <sndfile.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>
#include <cstdint>
#include <iostream>

namespace SoundImageConverter
{
	// Decodes a PNG image to a WAV file following specific metadata and pixel encoding rules
	bool Decoder::decode(const std::string& pngPath, const std::string& wavPath)
	{
		// Load PNG image
		int width, height, channels;
		unsigned char* image = stbi_load(pngPath.c_str(), &width, &height, &channels, 0);
		if (!image)
		{
			std::cerr << "Error: Could not open PNG file: " << pngPath << std::endl;
			return false;
		}

		// Extract metada from first row
		if (width < 6)
		{
			std::cerr << "Error: PNG width too small to contain metadata." << std::endl;
			stbi_image_free(image);
			return false;
		}
		uint32_t sampleRate = (static_cast<uint32_t>(image[0]) << 24) |
							  (static_cast<uint32_t>(image[1]) << 16) |
							  (static_cast<uint32_t>(image[2]) << 8) |
								static_cast<uint32_t>(image[3]);
		int numChannels = static_cast<int>(image[4]);
		int bitDepth = static_cast<int>(image[5]);
		if (numChannels < 1 || numChannels  > 2 || (bitDepth != 8 && bitDepth != 16))
		{
			std::cerr << "Error: Invalid metadata (channels: " << numChannels << ", bit depth: " << bitDepth << ")." << std::endl;
			stbi_image_free(image);
			return false;
		}

		// Calculate expected samples
		int channelsPerPixel = (bitDepth == 8) ? (numChannels == 1 ? 1 : 3) : 4;
		size_t totalPixels = static_cast<size_t>(width) * (height - 1); // Exclude metadata row
		size_t expectedSamples = totalPixels; // One pixel == one sample

		// Debug: Print metadata
		std::cout << "PNG Metadata: " << sampleRate << " Hz, " << numChannels << " channels, "
			<< bitDepth << "-bit" << std::endl;

		// Helper function to reverse sampleToPixel scaling
		auto pixelToSample = [](uint8_t pixel) -> int16_t
		{
				return static_cast<int16_t>(pixel) * 256 - 32768; // Reverse the scaling applied in Encoder
		};

		// Decode pixels to samples
		std::vector<int16_t> samples;
		size_t pixelIndex = width * channelsPerPixel; // Start after metadata row
		for (size_t i = 0; i < expectedSamples && pixelIndex < static_cast<size_t>(width * height * channelsPerPixel); i++)
		{
			if (channelsPerPixel == 1) // 8-bit mono (grayscale)
			{
				samples.push_back(pixelToSample(image[pixelIndex++]));
			}
			else if (channelsPerPixel == 3) // 8-bit stereo RGB
			{
				samples.push_back(pixelToSample(image[pixelIndex++]));     // Left channel (Red)
				samples.push_back(pixelToSample(image[pixelIndex++])); // Right channel (Green)
				pixelIndex++; // Skip Blue channel (constant value in Encoder)
			}
			else // 16-bit mono/stereo RGBA
			{
				int16_t left = pixelToSample(image[pixelIndex++]); // Red channel
				pixelIndex++; // Skip Green channel (half of Red in Encoder)
				if (numChannels == 2)
				{
					samples.push_back(left);
					int16_t right = pixelToSample(image[pixelIndex++]); // Blue channel
					samples.push_back(right);
					pixelIndex++; // Skip Alpha channel
				}
				else
				{
					samples.push_back(left);
					pixelIndex += 2; // Skip Alpha and Blue channels
				}
			}
		}

		stbi_image_free(image);

		// Debug: Print first few samples
		std::cout << "First 10 decoded samples: ";
		for (size_t i = 0; i < std::min<size_t>(10, samples.size()); i++)
		{
			std::cout << samples[i] << " ";
		}
		std::cout << std::endl;

		// Write WAV file
		SF_INFO sfInfo;
		sfInfo.frames = samples.size() / numChannels;
		sfInfo.samplerate = sampleRate;
		sfInfo.channels = numChannels;
		sfInfo.format = (bitDepth == 16 ? SF_FORMAT_WAV | SF_FORMAT_PCM_16 : SF_FORMAT_WAV | SF_FORMAT_PCM_U8);
		SNDFILE* audioFile = sf_open(wavPath.c_str(), SFM_WRITE, &sfInfo);
		if (!audioFile)
		{
			std::cerr << "Error: Could not open WAV file for writing: " << wavPath << std::endl;
			return false;
		}

		sf_count_t writeCount = sf_write_short(audioFile, samples.data(), samples.size());
		sf_close(audioFile);

		if (writeCount != static_cast<sf_count_t>(samples.size()))
		{
			std::cerr << "Error: Failed to write all samples to WAV file" << std::endl;
			return false;
		}

		std::cout << "Decoded " << pngPath << " to " << wavPath << std::endl;
		return true;
	}

} // namespace SoundImageConverter
