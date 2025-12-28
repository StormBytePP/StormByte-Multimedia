#pragma once

#include <StormByte/multimedia/metadata.hxx>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */

namespace StormByte::Multimedia {
	class File;
}

namespace StormByte::Multimedia::FFmpeg {
	class STORMBYTE_MULTIMEDIA_PRIVATE AVStream {
		friend class AVFormatContext;
		friend class StormByte::Multimedia::File;
		public:
			AVStream(const AVStream&) 							= delete;
			AVStream(AVStream&& other) noexcept;
			~AVStream() noexcept								= default;
			AVStream& operator=(const AVStream&) 				= delete;
			AVStream& operator=(AVStream&& other) noexcept;

			/**
			 * @brief Gets the index of the stream.
			 * @return int The index of the stream.
			 */
			int 												Index() const noexcept;

			/**
			 * @brief Gets the type of the stream.
			 * @return AVMediaType The type of the stream.
			 */
			AVMediaType 										Type() const noexcept;

			/**
			 * @brief Gets the codec parameters of the stream.
			 * @return const AVCodecParameters* The codec parameters.
			 */
			const AVCodecParameters* 							CodecParameters() const noexcept;

			/**
			 * @brief Gets the time base of the stream.
			 * @return AVRational The time base.
			 */
			AVRational 											TimeBase() const noexcept;

			/**
			 * @brief Get an estimated frame rate for the stream.
			 * @return double Frames per second (0.0 if unknown).
			 */
			double 												FrameRate() const noexcept;

			/**
			 * @brief Gets the metadata of the stream.
			 * @return Metadata The metadata object.
			 */
			Metadata											Metadata() const noexcept;

		private:
			::AVStream* m_stream = nullptr;

			/**
			 * @brief Private constructor.
			 * @param stream The raw AVStream pointer.
			 */
			explicit AVStream(::AVStream* stream) noexcept;
		};
}