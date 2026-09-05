#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>
// oldcode — Metadata se rehace
// #include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <optional>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVBSF;
	class AVCodecParameters;
	class AVPacket;
	class AVStream;

	/**
	 * @class AVFormatContext
	 * @brief RAII input format context (demuxer).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFormatContext: public AVPointer<::AVFormatContext> {
		public:
			/**
			 * @brief Copy constructor (deleted).
			 * @param other Unused.
			 */
			AVFormatContext(const AVFormatContext& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source context.
			 */
			AVFormatContext(AVFormatContext&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVFormatContext() noexcept override;

			/**
			 * @brief Copy assignment (deleted).
			 * @param other Unused.
			 * @return *this.
			 */
			AVFormatContext& operator=(const AVFormatContext& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source context.
			 * @return *this.
			 */
			AVFormatContext& operator=(AVFormatContext&& other) noexcept = default;

			/**
			 * @brief Opens a file and finds stream info.
			 * @param path Media path.
			 * @return Context or DecoderError.
			 */
			static ExpectedAVFormatContext Open(const std::filesystem::path& path);

			// oldcode — API pública Metadata
			// /**
			//  * @return Container metadata dictionary as Metadata.
			//  */
			// StormByte::Multimedia::Metadata Metadata() const noexcept;

			/**
			 * @brief Reads the next packet.
			 * @param packet Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReadPacket(AVPacket& packet) noexcept;

			/**
			 * @brief Non-owning stream views.
			 * @return Set of streams.
			 */
			Streams Streams() const noexcept;

			/**
			 * @brief Returns an mp4→Annex-B BSF when the container and codec require it.
			 * @param codec_id Codec id.
			 * @param stream_id Stream index (time base).
			 * @param params Codec parameters.
			 * @return BSF or nullopt.
			 */
			std::optional<AVBSF> Mp4ToAnnexB(int codec_id, int stream_id, const AVCodecParameters& params) const noexcept;

		private:
			/**
			 * @brief Adopts a raw format context.
			 * @param ctx Raw format context (owned).
			 */
			explicit AVFormatContext(::AVFormatContext* ctx) noexcept;

			/**
			 * @brief Closes input (avformat_close_input).
			 */
			void Free() noexcept override;
	};
}
