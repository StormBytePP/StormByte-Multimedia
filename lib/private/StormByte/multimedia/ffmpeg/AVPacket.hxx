#pragma once

#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class STORMBYTE_MULTIMEDIA_PRIVATE AVPacket {
		friend class AVDecoder;
		friend class AVFormatContext;
		public:
			/**
			 * @brief Private constructor.
			 */
			AVPacket() noexcept;
			AVPacket(const AVPacket& other)						= delete;
			AVPacket(AVPacket&& other) noexcept;
			~AVPacket() noexcept;
			AVPacket& operator=(const AVPacket& other)			= delete;
			AVPacket& operator=(AVPacket&& other) noexcept;

			/**
			 * @brief Unreferences the AVPacket.
			 */
			void 												Unref() noexcept;

			/**
			 * @brief Get the stream index of the underlying packet.
			 * @return int The stream index, or -1 if not available.
			 */
			int 												StreamIndex() const noexcept;

		private:
			::AVPacket* m_pkt;
	};
}