#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVCodecParameters
	 * @brief Wrapper class for FFmpeg's AVCodecParameters.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVCodecParameters: public AVPointer<::AVCodecParameters> {
		public:
			/**
			 * @brief Constructor with pointer.
			 * @param par The AVCodecParameters pointer to wrap.
			 */
			explicit AVCodecParameters(::AVCodecParameters* par) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other The other AVCodecParameters to copy from.
			 */
			AVCodecParameters(const AVCodecParameters& other) noexcept;

			/**
			 * @brief Move constructor.
			 * @param other The other AVCodecParameters to move from.
			 */
			AVCodecParameters(AVCodecParameters&& other) noexcept				= default;

			/**
			 * @brief Destructor.
			 */
			~AVCodecParameters() noexcept override;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other AVCodecParameters to copy from.
			 * @return Reference to this AVCodecParameters.
			 */
			AVCodecParameters& operator=(const AVCodecParameters& other) noexcept;

			/**
			 * @brief Move assignment operator.
			 * @param other The other AVCodecParameters to move from.
			 * @return Reference to this AVCodecParameters.
			 */
			AVCodecParameters& operator=(AVCodecParameters&& other) noexcept	= default;

		private:
			/**
			 * @brief Frees the underlying AVCodecParameters pointer.
			 */
			void 																Free() noexcept override;
		};
}
