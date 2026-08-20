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
 * @brief Internal FFmpeg wrappers.
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
			 * Copy constructor (deleted).
			 */
			AVFormatContext(const AVFormatContext& other) = delete;

			/**
			 * Move constructor.
			 */
			AVFormatContext(AVFormatContext&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVFormatContext() noexcept override;

			/**
			 * Copy assignment (deleted).
			 */
			AVFormatContext& operator=(const AVFormatContext& other) = delete;

			/**
			 * Move assignment.
			 */
			AVFormatContext& operator=(AVFormatContext&& other) noexcept = default;

			/**
			 * Opens a file and finds stream info.
			 * @param path Media path.
			 * @return Context or DecoderError.
			 */
			static ExpectedAVFormatContext Open(const std::filesystem::path& path);

			/**
			 * @return Container metadata dictionary as Metadata.
			 */
			StormByte::Multimedia::Metadata Metadata() const noexcept;

			/**
			 * Reads the next packet.
			 * @param packet Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReadPacket(AVPacket& packet) noexcept;

			/**
			 * @return Set of non-owning stream views.
			 */
			Streams Streams() const noexcept;

			/**
			 * Returns an mp4→Annex-B BSF when the container and codec require it.
			 * @param codec_id Codec id.
			 * @param stream_id Stream index (time base).
			 * @param params Codec parameters.
			 * @return BSF or nullopt.
			 */
			std::optional<AVBSF> Mp4ToAnnexB(int codec_id, int stream_id, const AVCodecParameters& params) const noexcept;

		private:
			/**
			 * @param ctx Raw format context (owned).
			 */
			explicit AVFormatContext(::AVFormatContext* ctx) noexcept;

			/**
			 * Closes input (avformat_close_input).
			 */
			void Free() noexcept override;
	};
}
