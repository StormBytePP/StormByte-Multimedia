#pragma once

#include <StormByte/multimedia/ffmpeg/AVBSFPipeline.hxx>
#include <StormByte/multimedia/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/ffmpeg/typedefs.hxx>

struct AVCodec;
struct AVCodecContext;

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVFormatContext;
	class AVFrame;
	class AVPacket;

	/**
	 * @class AVDecoder
	 * @brief Wrapper class for FFmpeg's AVCodecContext (Decoder).
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED AVDecoder: public AVPointer<::AVCodecContext> {
		public:
			/** 
			 * @brief Copy constructor (deleted).
			 * @param other The other AVDecoder to copy from.
			 */
			AVDecoder(const AVDecoder&) 									= delete;

			/** 
			 * @brief Move constructor.
			 * @param other The other AVDecoder to move from.
			 */
			AVDecoder(AVDecoder&& other) noexcept							= default;

			/** 
			 * @brief Destructor.
			 */
			~AVDecoder() noexcept override;

			/** 
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVDecoder to copy from.
			 * @return Reference to this AVDecoder.
			 */
			AVDecoder& operator=(const AVDecoder&) 							= delete;

			/** 
			 * @brief Move assignment operator.
			 * @param other The other AVDecoder to move from.
			 * @return Reference to this AVDecoder.
			 */
			AVDecoder& operator=(AVDecoder&& other) noexcept				= default;

			/**
			 * @brief Opens a decoder using codec parameters (calls avcodec_parameters_to_context()).
			 * @param codec The codec to use for decoding.
			 * @param params The codec parameters from the demuxer stream.
			 * @param stream_index The index of the stream these parameters came from.
			 * @param bsf_name Optional bitstream filter name to apply (e.g. "hevc_mp4toannexb").
			 * @return ExpectedAVDecoder The expected AVDecoder or a DecoderError.
			 */
			static ExpectedAVDecoder 										Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept;

			/**
			 * @brief Sends a packet to the decoder.
			 * @param pkt The packet to send.
			 * @return OperationResult The result of the send operation.
			 */
			FFmpeg::OperationResult 										SendPacket(AVPacket& pkt) noexcept;

			/**
			 * @brief Receives a frame from the decoder.
			 * @param frame The frame to receive into.
			 * @return OperationResult The result of the receive operation.
			 */
			FFmpeg::OperationResult 										ReceiveFrame(AVFrame& frame) noexcept;
			
			/**
			 * @brief Returns the stream index this decoder was opened for.
			 */
			int 															StreamIndex() const noexcept;

			/**
			 * @brief Flushes the decoder buffers (without draining frames).
			 */
			void 															Flush() noexcept;

			/**
			 * @brief Sets the decoder to end-of-file state so the last frames can be received.
			 */
			void 															SetEof() noexcept;

		private:
			int m_stream_index = -1;
			AVBSFPipeline m_bsf_pipeline;

			/**
			 * @brief Private constructor.
			 * @param ctx The codec context to use for decoding.
			 */
			explicit AVDecoder(AVCodecContext* ctx) noexcept;

			/**
			 * @brief Frees the underlying AVCodecContext pointer.
			 */
			void 															Free() noexcept override;
	};
}