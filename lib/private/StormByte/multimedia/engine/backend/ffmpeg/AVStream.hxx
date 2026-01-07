#pragma once

#include <StormByte/multimedia/metadata.hxx>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVCodecParameters;

	/**
	 * @class AVStream
	 * @brief Wrapper class for FFmpeg's AVStream.
	 * @note Memory for streams is not owned by this class; it is managed by AVFormatContext.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVStream {
		public:
			/**
			 * @brief Private constructor.
			 * @param stream The raw AVStream pointer.
			 */
			explicit AVStream(::AVStream* stream) noexcept;
			
			/**
			 * @brief Copy constructor (deleted).
			 * @param other The other AVStream to copy from.
			 */
			AVStream(const AVStream&) 							= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other AVStream to move from.
			 */
			AVStream(AVStream&& other) noexcept					= default;

			/**
			 * @brief Destructor.
			 */
			~AVStream() noexcept								= default;

			/**
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVStream to copy from.
			 * @return Reference to this AVStream.
			 */
			AVStream& operator=(const AVStream&) 				= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other AVStream to move from.
			 * @return Reference to this AVStream.
			 */
			AVStream& operator=(AVStream&& other) noexcept 		= default;

			/**
			 * @brief Less-than operator for comparing stream indices.
			 * @param other The other AVStream to compare with.
			 * @return true if this stream's index is less than the other's index.
			 */
			bool operator<(const AVStream& other) const noexcept;

			/**
			 * @brief Gets the index of the stream.
			 * @return int The index of the stream.
			 */
			int 												Index() const noexcept;

			/**
			 * @brief Gets the type of the stream.
			 * @return int The type of the stream.
			 */
			int 												Type() const noexcept;

			/**
			 * @brief Gets the codec parameters of the stream.
			 * @return const AVCodecParameters* The codec parameters.
			 */
			AVCodecParameters 									CodecParameters() const noexcept;

			/**
			 * @brief Gets the time base of the stream.
			 * @return AVRational The time base.
			 */
			AVRational 											TimeBase() const noexcept;

			/**
			 * @brief Get an estimated frame rate for the stream.
			 * @return double Frames per second (0.0 if unknown).
			 */
			double 												FrameRate() const noexcept;

			/**
			 * @brief Gets the metadata of the stream.
			 * @return Metadata The metadata object.
			 */
			StormByte::Multimedia::Metadata						Metadata() const noexcept;

		private:
			::AVStream* m_stream = nullptr;						///< Pointer to the underlying AVStream.
		};
}