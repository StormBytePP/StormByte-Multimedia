#pragma once

#include <StormByte/multimedia/ffmpeg/typedefs.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>

extern "C" {
	#include <libavformat/avformat.h>
}

namespace StormByte::Multimedia {
	class File;
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVPacket;
	struct StreamLess {
		bool operator()(const AVStream& a, const AVStream& b) const noexcept;
	};
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFormatContext {
		friend class StormByte::Multimedia::File;
		public:
			AVFormatContext(const AVFormatContext& other)						= delete;
			AVFormatContext(AVFormatContext&& other) noexcept;
			~AVFormatContext() noexcept;
			AVFormatContext& operator=(const AVFormatContext& other)			= delete;
			AVFormatContext& operator=(AVFormatContext&& other) noexcept;

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
			class Metadata 														Metadata() const noexcept;

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

		private:
			::AVFormatContext* m_ctx;

			/**
			 * @brief Private constructor.
			 * @param ctx The raw AVFormatContext pointer.
			 */
			explicit AVFormatContext(::AVFormatContext* ctx) noexcept;
	};
}