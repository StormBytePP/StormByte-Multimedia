#pragma once

#include <StormByte/multimedia/ffmpeg/AVPointer.hxx>

struct AVFrame;
struct AVFrameSideData;

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	/**
	 * @class AVFrame
	 * @brief Wrapper class for FFmpeg's AVFrame.
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED AVFrame: public AVPointer<::AVFrame> {
		friend class AVDecoder;
		friend class AVEncoder;
		public:
			/**
			 * @brief Default constructs an AVFrame and allocates the underlying ::AVFrame.
			 */
			AVFrame() noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 * @param other The other AVFrame to copy from.
			 */
			AVFrame(const AVFrame& other)						= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other AVFrame to move from.
			 */
			AVFrame(AVFrame&& other) noexcept 					= default;

			/**
			 * @brief Destructor.
			 */
			~AVFrame() noexcept override;

			/**
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVFrame to copy from.
			 * @return Reference to this AVFrame.
			 */
			AVFrame& operator=(const AVFrame& other)			= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other AVFrame to move from.
			 * @return Reference to this AVFrame.
			 */
			AVFrame& operator=(AVFrame&& other) noexcept		= default;

			/**
			 * @brief Unreferences the AVFrame.
			 */
			void 												Unref() noexcept;

			/**
			 * @brief Gets side data of the specified type from the AVFrame.
			 * @param type The type of side data to retrieve.
			 * @return const AVFrameSideData* Pointer to the side data, or nullptr if not found.
			 */
			const AVFrameSideData*								SideData(int type) const noexcept;

		private:
			/**
			 * @brief Frees the underlying AVFrame.
			 */
			void 												Free() noexcept override;
	};
}