#pragma once

#include <StormByte/multimedia/features.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace Codec
 * @brief Registry entry for logical codecs.
 */
namespace StormByte::Multimedia::Registry::Entry::Codec {
	/**
	 * @class Entry
	 * @brief Compile-time codec id + type + feature set.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Entry {
		public:
			/**
			 * @param id AVCodecID.
			 * @param type Media type.
			 * @param feature Single feature flag.
			 */
			constexpr Entry(AVCodecID id, enum Type type, Feature feature) noexcept
			: m_id(id), m_type(type), m_features(feature) {}

			/**
			 * @param id AVCodecID.
			 * @param type Media type.
			 * @param features Feature bitmask.
			 */
			constexpr Entry(AVCodecID id, enum Type type, Features features) noexcept
			: m_id(id), m_type(type), m_features(features) {}

			/**
			 * Copy constructor.
			 */
			constexpr Entry(const Entry& other) noexcept = default;

			/**
			 * Move constructor.
			 */
			constexpr Entry(Entry&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			constexpr ~Entry() noexcept = default;

			/**
			 * Copy assignment.
			 */
			constexpr Entry& operator=(const Entry& other) noexcept = default;

			/**
			 * Move assignment.
			 */
			constexpr Entry& operator=(Entry&& other) noexcept = default;

			/**
			 * @return Codec id.
			 */
			constexpr AVCodecID ID() const noexcept {
				return m_id;
			}

			/**
			 * @return Media type.
			 */
			constexpr enum Type Type() const noexcept {
				return m_type;
			}

			/**
			 * @return Feature set.
			 */
			[[nodiscard]] constexpr const class Features& Features() const noexcept {
				return m_features;
			}

		private:
			AVCodecID m_id;					///< Codec id
			enum Type m_type;				///< Media type
			class Features m_features;		///< Capabilities
	};
}
