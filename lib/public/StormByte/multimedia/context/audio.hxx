#pragma once

#include <StormByte/multimedia/context/generic.hxx>

#include <string>
#include <optional>

/**
 * @namespace Context
 * @brief The namespace for all context classes.
 */
namespace StormByte::Multimedia::Context {
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final: public Generic {
		public:
			/**
			 * @brief Default constructor.
			 */
			Audio(unsigned int sample_rate, unsigned short channels, unsigned int bitrate, const std::optional<std::string>& profile = std::nullopt) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other The other context to copy from.
			 */
			Audio(const Audio& other)								= default;

			/**
			 * @brief Move constructor.
			 * @param other The other context to move from.
			 */
			Audio(Audio&& other) noexcept							= default;

			/**
			 * @brief Default destructor.
			 */
			~Audio() noexcept 										= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other context to copy from.
			 * @return Reference to this context.
			 */
			Audio& operator=(const Audio& other)					= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other context to move from.
			 * @return Reference to this context.
			 */
			Audio& operator=(Audio&& other)							= default;

			/**
			 * @brief Gets the sample rate.
			 * @return The sample rate.
			 */
			unsigned int											SampleRate() const noexcept;

			/**
			 * @brief Gets the number of channels.
			 * @return The number of channels.
			 */
			unsigned short											Channels() const noexcept;

			/**
			 * @brief Gets the bitrate.
			 * @return The bitrate.
			 */
			unsigned int											Bitrate() const noexcept;

			/**
			 * @brief Gets the profile.
			 * @return The profile.
			 */
			const std::optional<std::string>&						Profile() const noexcept;

			/**
			 * @brief Clones the context.
			 * @return A pointer to the cloned context.
			 */
			PointerType 											Clone() const override;

			/**
			 * @brief Moves the context.
			 * @return A pointer to the moved context.
			 */
			PointerType 											Move() override;

		private:
			unsigned int m_sample_rate;								///< The sample rate
			unsigned short m_channels;								///< The number of channels
			unsigned int m_bitrate;									///< The bitrate
			std::optional<std::string> m_profile;					///< The profile (optional)
	};
}