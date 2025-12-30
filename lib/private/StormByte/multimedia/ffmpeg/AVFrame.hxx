#pragma once

#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavutil/frame.h>
}

namespace StormByte::Multimedia {
	class File;
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFrame {
		friend class AVBSF;
		friend class AVDecoder;
		friend class AVEncoder;
		friend class AVFormatContext;
		friend class StormByte::Multimedia::File;
		public:
			/**
			 * @brief Default constructs an AVFrame and allocates the underlying ::AVFrame.
			 */
			AVFrame() noexcept;
			AVFrame(const AVFrame& other)						= delete;
			AVFrame(AVFrame&& other) noexcept;
			~AVFrame() noexcept;
			AVFrame& operator=(const AVFrame& other)			= delete;
			AVFrame& operator=(AVFrame&& other) noexcept;

			/**
			 * @brief Unreferences the AVFrame.
			 */
			void 												Unref() noexcept;

			/**
			 * @brief Gets side data of the specified type from the AVFrame.
			 * @param type The type of side data to retrieve.
			 * @return const AVFrameSideData* Pointer to the side data, or nullptr if not found.
			 */
			const AVFrameSideData*								SideData(enum AVFrameSideDataType type) const noexcept;

		private:
			::AVFrame* m_frame;
	};
}