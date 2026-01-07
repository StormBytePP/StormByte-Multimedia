#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <optional>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVBSF;
	class AVCodecParameters;
	class AVPacket;
	class AVStream;

	/**
	 * @class AVFormatContext
	 * @brief Wrapper class for FFmpeg's AVFormatContext (Demuxer or Muxer).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFormatContext: public AVPointer<::AVFormatContext> {
		public:
			/** 
			 * @brief Copy constructor (deleted).
			 * @param other The other AVFormatContext to copy from.
			 */
			AVFormatContext(const AVFormatContext& other)						= delete;

			/** 
			 * @brief Move constructor.
			 * @param other The other AVFormatContext to move from.
			 */
			AVFormatContext(AVFormatContext&& other) noexcept					= default;

			/** 
			 * @brief Destructor.
			 */
			~AVFormatContext() noexcept override;

			/** 
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVFormatContext to copy from.
			 * @return Reference to this AVFormatContext.
			 */
			AVFormatContext& operator=(const AVFormatContext& other)			= delete;

			/** 
			 * @brief Move assignment operator.
			 * @param other The other AVFormatContext to move from.
			 * @return Reference to this AVFormatContext.
			 */
			AVFormatContext& operator=(AVFormatContext&& other) noexcept		= default;

			/**
			 * @brief Opens a multimedia file and returns the AVFormatContext.
			 * @param path The path to the multimedia file.
			 * @return ExpectedAVFormatContext The expected AVFormatContext or a DecoderError.
			 */
			static ExpectedAVFormatContext 										Open(const std::filesystem::path& path);

			/**
			 * @brief Gets the metadata of the AVFormatContext.
			 * @return Metadata The metadata object.
			 */
			StormByte::Multimedia::Metadata 									Metadata() const noexcept;

			/**
			 * @brief Reads a packet from the AVFormatContext.
			 * @param packet The packet to read into.
			 * @return OperationResult The result of the read operation.
			 */
			OperationResult														ReadPacket(AVPacket& packet) noexcept;

			/**
			 * @brief Gets the streams of the AVFormatContext.
			 * @return Streams The vector of AVStream objects.
			 */
			Streams																Streams() const noexcept;

			/**
			 * @brief Determines if a bitstream filter is needed for the given codec ID.
			 * @param codec_id The codec ID to check.
			 * @param stream_index The index of the stream to consider for time base.
			 * @param params The codec parameters to consider.
			 * @return std::optional<AVBSF> The bitstream filter if needed, std::nullopt otherwise.
			 */
			std::optional<AVBSF>												Mp4ToAnnexB(int codec_id, int stream_id, const AVCodecParameters& params) const noexcept;

		private:
			/**
			 * @brief Private constructor.
			 * @param ctx The raw AVFormatContext pointer.
			 */
			explicit AVFormatContext(::AVFormatContext* ctx) noexcept;

			/** 
			 * @brief Frees the underlying AVFormatContext pointer.
			 */
			void 																Free() noexcept override;
	};
}