#pragma once

#include <StormByte/multimedia/ffmpeg/typedefs.hxx>

extern "C" {
	#include <libavcodec/bsf.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVFrame;
	class AVPacket;

	/**
	 * @class AVBSF
	 * @brief Wrapper class for FFmpeg's AVBSFContext (Bitstream Filter).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVBSF {
		public:
			AVBSF(const AVBSF&) = delete;
			AVBSF& operator=(const AVBSF&) = delete;

			AVBSF(AVBSF&& other) noexcept;
			AVBSF& operator=(AVBSF&& other) noexcept;

			~AVBSF() noexcept;

			static ExpectedAVBSF Create(const char* name, AVCodecParameters* params, AVRational time_base) noexcept;

			OperationResult SendPacket(const AVPacket& pkt) noexcept;
			OperationResult ReceivePacket(AVPacket& pkt) noexcept;

			void Flush() noexcept;
			void SetEof() noexcept;

		private:
			explicit AVBSF(AVBSFContext* ctx) noexcept;

			AVBSFContext* m_ctx = nullptr;
		};
}