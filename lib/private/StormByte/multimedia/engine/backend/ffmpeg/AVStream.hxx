#pragma once

// oldcode — Metadata se rehace; no incluir lib/old
// #include <StormByte/multimedia/metadata.hxx>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVCodecParameters;

	/**
	 * @class AVStream
	 * @brief Non-owning view of an ::AVStream (owned by AVFormatContext).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVStream {
		public:
			/**
			 * @brief Binds a raw stream pointer (not owned).
			 * @param stream Raw stream pointer.
			 */
			explicit AVStream(::AVStream* stream) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			AVStream(const AVStream&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source view.
			 */
			AVStream(AVStream&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVStream() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			AVStream& operator=(const AVStream&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source view.
			 * @return *this.
			 */
			AVStream& operator=(AVStream&& other) noexcept = default;

			/**
			 * @brief Orders by stream index (for std::set).
			 * @param other Other stream.
			 * @return true if this index is less.
			 */
			bool operator<(const AVStream& other) const noexcept;

			/**
			 * @brief Stream index.
			 * @return Index, or -1.
			 */
			int Index() const noexcept;

			/**
			 * @brief Stream media type.
			 * @return AVMediaType as int.
			 */
			int Type() const noexcept;

			/**
			 * @brief Copy of codec parameters.
			 * @return Parameters wrapper.
			 */
			AVCodecParameters CodecParameters() const noexcept;

			/**
			 * @brief Stream time base.
			 * @return Time base.
			 */
			AVRational TimeBase() const noexcept;

			/**
			 * @brief Estimated FPS (avg or r_frame_rate).
			 * @return FPS, or 0.
			 */
			double FrameRate() const noexcept;

			// oldcode — API pública Metadata
			// /**
			//  * @return Stream-level metadata tags.
			//  */
			// StormByte::Multimedia::Metadata Metadata() const noexcept;

		private:
			::AVStream* m_stream = nullptr;	///< Non-owning
	};
}
