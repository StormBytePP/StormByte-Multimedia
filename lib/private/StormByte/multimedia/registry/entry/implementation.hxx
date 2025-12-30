#pragma once

#include <StormByte/multimedia/features.hxx>

/**
 * @namespace Implementation
 * @brief The namespace for registering decoders and encoders entries.
 */
namespace StormByte::Multimedia::Registry::Entry::Implementation {
	/**
	 * @class Entry
	 * @brief Represents a registry entry for an encoder/decoder.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Entry {
		public:
			/**
			 * @brief Construct an entry from a name and a single Feature.
			 * @param name Human-readable name.
			 * @param feature Base feature.
			 */
			constexpr Entry(const char* name, Feature feature) noexcept
			:m_name(name), m_features(feature) {}

			/**
			 * @brief Construct an entry from a name and a Features bitmask.
			 * @param name Human-readable name.
			 * @param features Base features.
			 */
			constexpr Entry(const char* name, Features features) noexcept
			:m_name(name), m_features(features) {}

			/**
			 * @brief Default copy constructor.
			 * @param other The other entry to copy from.
			 */
			constexpr Entry(const Entry& other) noexcept 				= default;

			/**
			 * @brief Default move constructor.
			 * @param other The other entry to move from.
			 */
			constexpr Entry(Entry&& other) noexcept 					= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr ~Entry() noexcept 								= default;

			/**
			 * @brief Default copy assignment operator.
			 * @param other The other entry to copy from.
			 * @return Entry& Reference to this entry.
			 */
			constexpr Entry& operator=(const Entry& other) noexcept 	= default;

			/**
			 * @brief Default move assignment operator.
			 * @param other The other entry to move from.
			 * @return Entry& Reference to this entry.
			 */
			constexpr Entry& operator=(Entry&& other) noexcept 			= default;

			/**
			 * @brief Get the entry name.
			 */
			[[nodiscard]] constexpr const char* 						Name() const noexcept {
				return m_name;
			}

			/**
			 * @brief Get the feature set.
			 */
			[[nodiscard]] constexpr const class Features& 				Features() const noexcept {
				return m_features;
			}

		private:
			const char* m_name;											///< Human-readable name
			class Features m_features;									///< Supported features
	};
}