#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVCodecParameters
	 * @brief Deep-copying RAII wrapper for ::AVCodecParameters.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVCodecParameters: public AVPointer<::AVCodecParameters> {
		public:
			/**
			 * Allocates and optionally copies from @p par.
			 * @param par Source parameters (may be null).
			 */
			explicit AVCodecParameters(::AVCodecParameters* par) noexcept;

			/**
			 * Copy constructor (deep copy).
			 */
			AVCodecParameters(const AVCodecParameters& other) noexcept;

			/**
			 * Move constructor.
			 */
			AVCodecParameters(AVCodecParameters&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVCodecParameters() noexcept override;

			/**
			 * Copy assignment (deep copy).
			 */
			AVCodecParameters& operator=(const AVCodecParameters& other) noexcept;

			/**
			 * Move assignment.
			 */
			AVCodecParameters& operator=(AVCodecParameters&& other) noexcept = default;

		private:
			/**
			 * Frees parameters (avcodec_parameters_free).
			 */
			void Free() noexcept override;
	};
}
