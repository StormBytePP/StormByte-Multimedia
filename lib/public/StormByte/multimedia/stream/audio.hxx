#pragma once

#include <StormByte/multimedia/media/property/audio/channels.hxx>
#include <StormByte/multimedia/media/property/audio/sample.hxx>
#include <StormByte/multimedia/stream/base.hxx>

#include <memory>
#include <string>

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
			Audio(const Audio& stream) noexcept 							= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Audio(Audio&& stream) noexcept									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			Audio& operator=(const Audio& stream) noexcept 					= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			Audio& operator=(Audio&& stream) noexcept 						= default;

			/**
			 * @brief Destructor.
			 */
			~Audio() noexcept override										= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 													Clone() const override;

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 													Move() override;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			constexpr Media::Type 											Type() const noexcept {
				return Media::Type::Audio;
			}

			/**
			 * @brief Gets the number of channels.
			 * @return The number of channels.
			 */
			std::shared_ptr<const Media::Property::Audio::Channels>&		Channels() noexcept;

			/**
			 * @brief Gets the number of channels.
			 * @return The number of channels.
			 */
			const std::shared_ptr<const Media::Property::Audio::Channels>&	Channels() const noexcept;

			/**
			 * @brief Gets the sample rate.
			 * @return The sample rate.
			 */
			std::shared_ptr<const Media::Property::Audio::Sample>& 			Sample() noexcept;

			/**
			 * @brief Gets the sample rate.
			 * @return The sample rate.
			 */
			const std::shared_ptr<const Media::Property::Audio::Sample>& 	Sample() const noexcept;

			/**
			 * @brief Gets the bitrate.
			 * @return The bitrate.
			 */
			std::shared_ptr<const unsigned int>& 							Bitrate() noexcept;

			/**
			 * @brief Gets the bitrate.
			 * @return The bitrate.
			 */
			const std::shared_ptr<const unsigned int>& 						Bitrate() const noexcept;

			/**
			 * @brief Gets the profile.
			 * @return The profile.
			 */
			std::shared_ptr<const std::string>& 							Profile() noexcept;

			/**
			 * @brief Gets the profile.
			 * @return The profile.
			 */
			const std::shared_ptr<const std::string>& 						Profile() const noexcept;

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			Audio(std::shared_ptr<const Multimedia::Codec> codec) noexcept;

			std::shared_ptr<const Media::Property::Audio::Channels> m_channels;	///< The number of channels.
			std::shared_ptr<const Media::Property::Audio::Sample> m_sample;		///< The sample rate.
			std::shared_ptr<const unsigned int> m_bitrate;						///< The number of bits per sample.
			std::shared_ptr<const std::string> m_profile;						///< The profile.
	};
}