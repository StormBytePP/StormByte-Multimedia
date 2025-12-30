#pragma once

#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

namespace StormByte::Multimedia {
	class File;
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class STORMBYTE_MULTIMEDIA_PRIVATE AVCodecParameters {
		friend class AVBSF;
		friend class AVDecoder;
		friend class AVEncoder;
		friend class AVStream;
		friend class StormByte::Multimedia::File;
		public:
			AVCodecParameters() noexcept;
			AVCodecParameters(const AVCodecParameters& other) noexcept;
			AVCodecParameters(AVCodecParameters&& other) noexcept;
			~AVCodecParameters() noexcept;
			AVCodecParameters& operator=(const AVCodecParameters& other) noexcept;
			AVCodecParameters& operator=(AVCodecParameters&& other) noexcept;

		private:
			::AVCodecParameters* m_par = nullptr;

			explicit AVCodecParameters(::AVCodecParameters* par) noexcept;
		};
}
