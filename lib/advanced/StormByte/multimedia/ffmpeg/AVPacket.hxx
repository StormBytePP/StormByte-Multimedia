#pragma once

#include <StormByte/multimedia/ffmpeg/AVPointer.hxx>

struct AVPacket;

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	/**
	 * @class AVPacket
	 * @brief Wrapper class for FFmpeg's AVPacket.
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED AVPacket: public AVPointer<::AVPacket*> {
		friend class AVBSF;
		friend class AVEncoder;
		public:
			/**
			 * @brief Constructor initializing an empty AVPacket.
			 */
			AVPacket() noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 * @param other The other AVPacket to copy from.
			 */
			AVPacket(const AVPacket& other)						= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other AVPacket to move from.
			 */
			AVPacket(AVPacket&& other) noexcept 				= default;

			/**
			 * @brief Destructor.
			 */
			~AVPacket() noexcept override;

			/**
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVPacket to copy from.
			 * @return Reference to this AVPacket.
			 */
			AVPacket& operator=(const AVPacket& other)			= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other AVPacket to move from.
			 * @return Reference to this AVPacket.
			 */
			AVPacket& operator=(AVPacket&& other) noexcept 		= default;

			/**
			 * @brief Creates a reference to the underlying AVPacket.
			 * @return AVPacket A new AVPacket that references the same underlying packet.
			 */
			FFmpeg::AVPacket 									Ref() const noexcept;

			/**
			 * @brief Unreferences the AVPacket.
			 */
			void 												Unref() noexcept;

			/**
			 * @brief Get the stream index of the underlying packet.
			 * @return int The stream index, or -1 if not available.
			 */
			int 												StreamIndex() const noexcept;

		private:
			/**
			 * @brief Frees the underlying AVPacket.
			 */
			void 												Free() noexcept override;
	};
}