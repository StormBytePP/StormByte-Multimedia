#pragma once

#include <StormByte/multimedia/ffmpeg/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

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
	class STORMBYTE_MULTIMEDIA_PRIVATE AVDecoder {
	public:
		AVDecoder(const AVDecoder&) = delete;
		AVDecoder(AVDecoder&& other) noexcept;
		~AVDecoder() noexcept;
		AVDecoder& operator=(const AVDecoder&) = delete;
		AVDecoder& operator=(AVDecoder&& other) noexcept;

		/**
		 * @brief Opens a decoder for the given codec.
		 * @param codec The codec to use for decoding.
		 * @return ExpectedAVDecoder The expected AVDecoder or a DecoderError.
		 */
		static ExpectedAVDecoder 							Open(AVCodec* codec) noexcept;

		/**
		 * @brief Opens a decoder using codec parameters (calls avcodec_parameters_to_context()).
		 * @param codec The codec to use for decoding.
		 * @param params The codec parameters from the demuxer stream.
		 * @param stream_index The index of the stream these parameters came from.
		 * @param bsf_name Optional bitstream filter name to apply (e.g. "hevc_mp4toannexb").
		 * @return ExpectedAVDecoder The expected AVDecoder or a DecoderError.
		 */
		static ExpectedAVDecoder 							Open(AVCodec* codec, AVCodecParameters* params, int stream_index, const char* bsf_name = nullptr) noexcept;

		/**
		 * @brief Sends a packet to the decoder.
		 * @param pkt The packet to send.
		 * @return OperationResult The result of the send operation.
		 */
		FFmpeg::OperationResult 							SendPacket(const AVPacket& pkt) noexcept;

		/**
		 * @brief Receives a frame from the decoder.
		 * @param frame The frame to receive into.
		 * @return OperationResult The result of the receive operation.
		 */
		FFmpeg::OperationResult 							ReceiveFrame(AVFrame& frame) noexcept;
		
		/**
		 * @brief Returns the stream index this decoder was opened for.
		 */
		int 												StreamIndex() const noexcept;

		/**
		 * @brief Flushes the decoder buffers (without draining frames).
		 */
		void 												Flush() noexcept;

		/**
		 * @brief Sets the decoder to end-of-file state so the last frames can be received.
		 */
		void 												SetEof() noexcept;

	private:
		AVCodecContext* m_ctx = nullptr;
		int m_stream_index = -1;
		AVBSFContext* m_bsf = nullptr;

		/**
		 * @brief Private constructor.
		 * @param ctx The codec context to use for decoding.
		 */
		explicit AVDecoder(AVCodecContext* ctx) noexcept;
	};
}