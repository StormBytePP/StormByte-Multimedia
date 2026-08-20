#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/bsf.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVCodecParameters;
	class AVPacket;

	/**
	 * @class AVBSF
	 * @brief RAII bitstream filter context.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVBSF: public AVPointer<::AVBSFContext> {
		public:
			/**
			 * Copy constructor (deleted).
			 */
			AVBSF(const AVBSF&) = delete;

			/**
			 * Move constructor.
			 */
			AVBSF(AVBSF&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVBSF() noexcept override;

			/**
			 * Copy assignment (deleted).
			 */
			AVBSF& operator=(const AVBSF&) = delete;

			/**
			 * Move assignment.
			 */
			AVBSF& operator=(AVBSF&& other) noexcept;

			/**
			 * Creates and initializes a named BSF.
			 * @param name Filter name (e.g. "h264_mp4toannexb").
			 * @param params Input codec parameters.
			 * @param time_base Input time base.
			 * @return AVBSF or BSFError.
			 */
			static ExpectedAVBSF Create(const std::string& name, const AVCodecParameters& params, AVRational time_base) noexcept;

			/**
			 * Sends a packet into the filter.
			 * @param pkt Packet to send.
			 * @return Operation result.
			 */
			OperationResult SendPacket(AVPacket& pkt) noexcept;

			/**
			 * Receives a filtered packet.
			 * @param pkt Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReceivePacket(AVPacket& pkt) noexcept;

			/**
			 * Flushes the filter.
			 */
			void Flush() noexcept;

			/**
			 * Signals EOF (null packet).
			 */
			void SetEof() noexcept;

		private:
			/**
			 * @param ctx Allocated BSF context.
			 */
			explicit AVBSF(AVBSFContext* ctx) noexcept;

			/**
			 * Frees the BSF (av_bsf_free).
			 */
			void Free() noexcept override;
	};
}
