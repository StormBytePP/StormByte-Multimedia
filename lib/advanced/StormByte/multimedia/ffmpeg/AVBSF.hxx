#pragma once

#include <StormByte/multimedia/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/ffmpeg/typedefs.hxx>

struct AVBSFContext;
struct AVRational;

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVCodecParameters;
	class AVPacket;

	/**
	 * @class AVBSF
	 * @brief Wrapper class for FFmpeg's AVBSFContext (Bitstream Filter).
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED AVBSF: public AVPointer<::AVBSFContext*> {
		public:
			/**
			 * @brief Copy constructor (deleted).
			 * @param other The other AVBSF to copy from.
			 */
			AVBSF(const AVBSF&) 								= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other AVBSF to move from.
			 */
			AVBSF(AVBSF&& other) noexcept						= default;

			/**
			 * @brief Destructor.
			 */
			~AVBSF() noexcept override;

			/**
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVBSF to copy from.
			 * @return Reference to this AVBSF.
			 */
			AVBSF& operator=(const AVBSF&) 						= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other AVBSF to move from.
			 * @return Reference to this AVBSF.
			 */
			AVBSF& operator=(AVBSF&& other) noexcept;

			/**
			 * @brief Creates and initializes a bitstream filter context.
			 * @param name The name of the bitstream filter.
			 * @param params The codec parameters to initialize the filter with.
			 * @param time_base The time base for the filter.
			 * @return ExpectedAVBSF The created AVBSF or an error.
			 */
			static ExpectedAVBSF 								Create(const std::string& name, const AVCodecParameters& params, AVRational time_base) noexcept;

			/**
			 * @brief Sends a packet to the bitstream filter.
			 * @param pkt The packet to send.
			 * @return OperationResult The result of the operation.
			 */
			OperationResult 									SendPacket(const AVPacket& pkt) noexcept;

			/**
			 * @brief Receives a filtered packet from the bitstream filter.
			 * @param pkt The packet to receive into.
			 * @return OperationResult The result of the operation.
			 */
			OperationResult 									ReceivePacket(AVPacket& pkt) noexcept;

			/**
			 * @brief Flushes the bitstream filter.
			 */
			void 												Flush() noexcept;

			/**
			 * @brief Sets the end-of-file flag for the bitstream filter.
			 */
			void 												SetEof() noexcept;

		private:
			/**
			 * @brief Private constructor.
			 * @param ctx The AVBSFContext pointer to wrap.
			 */
			explicit AVBSF(AVBSFContext* ctx) noexcept;

			/**
			 * @brief Frees the underlying AVBSFContext.
			 */
			void 												Free() noexcept override;
		};
}