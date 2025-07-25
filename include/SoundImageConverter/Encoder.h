#ifndef SOUNDIMAGECONVERTER_ENCODER_H
#define SOUNDIMAGECONVERTER_ENCODER_H

#include <string>

namespace SoundImageConverter
{
	class Encoder
	{
	public:
		// Encodes a WAAV file to a PNG image
		// Returns true if successful, false otherwise
		static bool encode(const std::string& wavPath, const std::string& pngPath);
	};
}

#endif // SOUNDIMAGECONVERTER_ENCODER_H