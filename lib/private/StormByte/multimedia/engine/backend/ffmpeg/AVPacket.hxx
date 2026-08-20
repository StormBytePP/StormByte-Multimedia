#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>

extern "C" {
	#include <libavcodec/packet.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVPacket
	 * @brief RAII wrapper for ::AVPacket.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVPacket: public AVPointer<::AVPacket> {
		friend class AVBSF;
		friend class AVDecoder;
		friend class AVEncoder;
		friend class AVFormatContext;
	public:
		/**
		 * Allocates an empty packet.
		 */
		AVPacket() noexcept;

		/**
		 * Copy constructor (deleted).
		 */
		AVPacket(const AVPacket& other) = delete;

		/**
		 * Move constructor.
		 */
		AVPacket(AVPacket&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~AVPacket() noexcept override;

		/**
		 * Copy assignment (deleted).
		 */
		AVPacket& operator=(const AVPacket& other) = delete;

		/**
		 * Move assignment.
		 */
		AVPacket& operator=(AVPacket&& other) noexcept = default;

		/**
		 * @return New packet referencing the same data (av_packet_ref).
		 */
		FFmpeg::AVPacket Ref() const noexcept;

		/**
		 * Unreferences packet data (av_packet_unref).
		 */
		void Unref() noexcept;

		/**
		 * @return stream_index, or -1 if empty.
		 */
		int StreamIndex() const noexcept;

	private:
		/**
		 * Frees the packet (av_packet_free).
		 */
		void Free() noexcept override;
	};
}
