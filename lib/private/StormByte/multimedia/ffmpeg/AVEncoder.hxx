#pragma once

#include <StormByte/multimedia/ffmpeg/AVBSFPipeline.hxx>
#include <StormByte/multimedia/ffmpeg/typedefs.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
class AVFormatContext;
class AVFrame;
class AVPacket;

/**
 * @class AVEncoder
 * @brief The class representing an FFmpeg encoder.
 */
class STORMBYTE_MULTIMEDIA_PRIVATE AVEncoder {
	public:
		/**
		 * @brief Copy constructor (deleted).
		 * @param other The other encoder to copy from.
		 */
		AVEncoder(const AVEncoder& other) = delete;

		/**
		 * @brief Move constructor.
		 * @param other The other encoder to move from.
		 */
		AVEncoder(AVEncoder&& other) noexcept;

		/**
		 * @brief Destructor.
		 */
		~AVEncoder() noexcept;

		/**
		 * @brief Copy assignment operator (deleted).
		 * @param other The other encoder to copy from.
		 * @return Reference to this encoder.
		 */
		AVEncoder& operator=(const AVEncoder&) = delete;

		/**
		 * @brief Move assignment operator.
		 * @param other The other encoder to move from.
		 * @return Reference to this encoder.
		 */
		AVEncoder& operator=(AVEncoder&& other) noexcept;

		/**
		 * @brief Opens an encoder for the given codec.
		 * @param codec The codec to use for encoding.
		 * @return ExpectedAVEncoder The expected AVEncoder or an EncoderError.
		 */
		static ExpectedAVEncoder Open(AVCodec* codec) noexcept;

		/**
		 * @brief Opens an encoder using codec parameters (calls avcodec_parameters_to_context()).
		 * @param codec The codec to use for encoding.
		 * @param params The codec parameters from the muxer stream.
		 * @param stream_index The index of the stream these parameters came from.
		 * @param bsf_name Optional bitstream filter name to apply (e.g. "hevc_mp4toannexb").
		 * @return ExpectedAVEncoder The expected AVEncoder or an EncoderError.
		 */
		static ExpectedAVEncoder Open(AVCodec* codec, AVCodecParameters* params, const FFmpeg::AVFormatContext& fmt, int stream_index) noexcept;

		/**
		 * @brief Sends a frame to the encoder.
		 * @param frame The frame to send.
		 * @return OperationResult The result of the send operation.
		 */
		FFmpeg::OperationResult SendFrame(const AVFrame& frame) noexcept;

		/**
		 * @brief Receives a packet from the encoder.
		 * @param pkt The packet to receive into.
		 * @return OperationResult The result of the receive operation.
		 */
		FFmpeg::OperationResult ReceivePacket(AVPacket& pkt) noexcept;

		/**
		 * @brief Returns the stream index this encoder was opened for.
		 */
		int StreamIndex() const noexcept;

		/**
		 * @brief Flushes the encoder buffers.
		 */
		void Flush() noexcept;

		/**
		 * @brief Sets the encoder to end-of-file state.
		 */
		void SetEof() noexcept;

	private:
		AVCodecContext* m_ctx = nullptr;
		int m_stream_index = -1;
		FFmpeg::AVBSFPipeline m_bsf_pipeline;

		explicit AVEncoder(AVCodecContext* ctx) noexcept;
	};
}