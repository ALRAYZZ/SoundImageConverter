#ifndef SOUNDIMAGECONVERTER_ENCODER_H
#define SOUNDIMAGECONVERTER_ENCODER_H

#include <string>

namespace SoundImageConverter
{
	class Encoder
	{
	public:
		// Encodes a WAV file to a PNG image
		// Returns true if successful, false otherwise
		static bool encode(const std::string& wavPath, const std::string& pngPath);
	};

	class Decoder
	{
	public:
		// Decodes a PNG image to a WAV file
		static bool decode(const std::string& pngPath, const std::string& wavPath);
	};
}

#endif // SOUNDIMAGECONVERTER_ENCODER_H