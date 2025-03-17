#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>

/**
 * @namespace Audio
 * @brief The namespace for multimedia audio media properties.
 */
namespace StormByte::Multimedia::Media::Property::Audio {
	class STORMBYTE_MULTIMEDIA_PUBLIC Channels {
		public:
			/**
			 * @brief Constructor.
			 * @param number The number of channels.
			 * @param layout The channel layout.
			 */
			Channels(const unsigned short& number, const std::string& layout) noexcept;

			/**
			 * @brief Constructor.
			 * @param number The number of channels.
			 * @param layout The channel layout.
			 */
			Channels(unsigned short&& number, std::string&& layout) noexcept;

			/**
			 * @brief Copy constructor
			 * @param channels The Channels to copy.
			 */
			Channels(const Channels& channels)					= default;

			/**
			 * @brief Move constructor.
			 * @param channels The Channels to move.
			 */
			Channels(Channels&& channels) noexcept				= default;

			/**
			 * Copy assignment operator.
			 * @param channels The Channels to copy.
			 * @return The copied Channels.
			 */
			Channels& operator=(const Channels& channels)		= default;

			/**
			 * Move assignment operator.
			 * @param channels The Channels to move.
			 * @return The moved Channels.
			 */
			Channels& operator=(Channels&& channels) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			~Channels() noexcept								= default;

			/**
			 * Gets the number of channels
			 * @return The number of channels
			 */
			const unsigned short& 								Number() const noexcept;

			/**
			 * Gets the channel layout
			 * @return The channel layout
			 */
			const std::string& 									Layout() const noexcept;

		private:
			unsigned short m_number;							///< The number of channels.
			std::string m_layout;								///< The channel layout.
	};
}