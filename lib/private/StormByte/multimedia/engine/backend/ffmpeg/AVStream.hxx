#pragma once

// oldcode — Metadata se rehace; no incluir lib/old
// #include <StormByte/multimedia/metadata.hxx>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
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
			 * @param stream Raw stream pointer (not owned).
			 */
			explicit AVStream(::AVStream* stream) noexcept;

			/**
			 * Copy constructor (deleted).
			 */
			AVStream(const AVStream&) = delete;

			/**
			 * Move constructor.
			 */
			AVStream(AVStream&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVStream() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			AVStream& operator=(const AVStream&) = delete;

			/**
			 * Move assignment.
			 */
			AVStream& operator=(AVStream&& other) noexcept = default;

			/**
			 * Orders by stream index (for std::set).
			 * @param other Other stream.
			 * @return true if this index is less.
			 */
			bool operator<(const AVStream& other) const noexcept;

			/**
			 * @return Stream index, or -1.
			 */
			int Index() const noexcept;

			/**
			 * @return AVMediaType as int.
			 */
			int Type() const noexcept;

			/**
			 * @return Copy of codec parameters.
			 */
			AVCodecParameters CodecParameters() const noexcept;

			/**
			 * @return Stream time base.
			 */
			AVRational TimeBase() const noexcept;

			/**
			 * @return Estimated FPS (avg or r_frame_rate), or 0.
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
