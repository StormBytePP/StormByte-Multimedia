#pragma once

#include <StormByte/multimedia/media/tags.hxx>
#include <StormByte/multimedia/stream/base.hxx>

/**
 * @namespace Stream
 * @brief The namespace for all multimedia streams types.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Audio
	 * @brief Class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final: public Base {
		friend class Base;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Audio(const Audio& stream) noexcept 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Audio(Audio&& stream) noexcept								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			Audio& operator=(const Audio& stream) noexcept 				= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			Audio& operator=(Audio&& stream) noexcept 					= default;

			/**
			 * @brief Destructor.
			 */
			~Audio() noexcept override									= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 												Clone() const override;

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 												Move() override;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			constexpr Media::Type 										Type() const noexcept {
				return Media::Type::Audio;
			}

			/**
			 * @brief Gets the number of channels.
			 * @return The number of channels.
			 */
			unsigned short& 											Channels() noexcept;

			/**
			 * @brief Gets the number of channels.
			 * @return The number of channels.
			 */
			const unsigned short& 										Channels() const noexcept;

			/**
			 * @brief Gets the sample rate.
			 * @return The sample rate.
			 */
			unsigned int& 												SampleRate() noexcept;

			/**
			 * @brief Gets the sample rate.
			 * @return The sample rate.
			 */
			const unsigned int& 										SampleRate() const noexcept;

			/**
			 * @brief Gets the channel layout.
			 * @return The channel layout.
			 */
			std::string& 												ChannelLayout() noexcept;

			/**
			 * @brief Gets the channel layout.
			 * @return The channel layout.
			 */
			const std::string& 											ChannelLayout() const noexcept;

			/**
			 * @brief Gets the bitrate.
			 * @return The bitrate.
			 */
			unsigned int& 												Bitrate() noexcept;

			/**
			 * @brief Gets the bitrate.
			 * @return The bitrate.
			 */
			const unsigned int& 										Bitrate() const noexcept;

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			Audio(const Media::Codec::Name& codec) noexcept;

			unsigned short m_channels;									///< The number of channels.
			unsigned int m_sample_rate;									///< The sample rate.
			std::string m_channel_layout;								///< The channel layout.
			unsigned int m_bitrate;										///< The number of bits per sample.
	};
}