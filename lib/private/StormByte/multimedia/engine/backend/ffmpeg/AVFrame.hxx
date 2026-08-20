#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>

extern "C" {
	#include <libavutil/frame.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVFrame
	 * @brief RAII wrapper for ::AVFrame.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFrame: public AVPointer<::AVFrame> {
		friend class AVDecoder;
		friend class AVEncoder;
	public:
		/**
		 * Allocates an empty frame.
		 */
		AVFrame() noexcept;

		/**
		 * Copy constructor (deleted).
		 */
		AVFrame(const AVFrame& other) = delete;

		/**
		 * Move constructor.
		 */
		AVFrame(AVFrame&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~AVFrame() noexcept override;

		/**
		 * Copy assignment (deleted).
		 */
		AVFrame& operator=(const AVFrame& other) = delete;

		/**
		 * Move assignment.
		 */
		AVFrame& operator=(AVFrame&& other) noexcept = default;

		/**
		 * Unreferences frame buffers (av_frame_unref).
		 */
		void Unref() noexcept;

		/**
		 * @param type AVFrameSideDataType value.
		 * @return Side data pointer, or nullptr.
		 */
		const AVFrameSideData* SideData(int type) const noexcept;

	private:
		/**
		 * Frees the frame (av_frame_free).
		 */
		void Free() noexcept override;
	};
}
