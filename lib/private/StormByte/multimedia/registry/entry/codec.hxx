#pragma once

#include <StormByte/multimedia/features.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace Codec
 * @brief The namespace for registering codecs entries.
 */
namespace StormByte::Multimedia::Registry::Entry::Codec {
	/**
	 * @class Entry
	 * @brief Represents a registry entry for a codec.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Entry {
		public:
			/**
			 * @brief Construct an entry from a name and a single Feature.
			 * @param id Codec ID.
			 * @param type Codec type.
			 * @param feature Base feature.
			 */
			constexpr Entry(AVCodecID id, enum Type type, Feature feature) noexcept
			:m_id(id), m_type(type), m_features(feature) {}

			/**
			 * @brief Construct an entry from a name and a Features bitmask.
			 * @param id Codec ID.
			 * @param type Codec type.
			 * @param features Base features.
			 */
			constexpr Entry(AVCodecID id, enum Type type, Features features) noexcept
			:m_id(id), m_type(type), m_features(features) {}

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
			constexpr AVCodecID 										ID() const noexcept {
				return m_id;
			}

			/**
			 * @brief Get the codec type.
			 */
			constexpr enum Type 										Type() const noexcept {
				return m_type;
			}

			/**
			 * @brief Get the feature set.
			 */
			[[nodiscard]] constexpr const class Features& 				Features() const noexcept {
				return m_features;
			}

		private:
			AVCodecID m_id;												///< Codec ID
			enum Type m_type;											///< Codec type
			class Features m_features;									///< Supported features
	};
}