#pragma once

#include <StormByte/multimedia/features.hxx>

/**
 * @namespace Implementation
 * @brief Registry entry for named decoder/encoder implementations.
 */
namespace StormByte::Multimedia::Registry::Entry::Implementation {
	/**
	 * @class Entry
	 * @brief Compile-time implementation name + feature set.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Entry {
		public:
			/**
			 * @param name Implementation name (e.g. "libx264").
			 * @param feature Single feature.
			 */
			constexpr Entry(const char* name, Feature feature) noexcept
			: m_name(name), m_features(feature) {}

			/**
			 * @param name Implementation name.
			 * @param features Feature bitmask.
			 */
			constexpr Entry(const char* name, Features features) noexcept
			: m_name(name), m_features(features) {}

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
			 * @return Implementation name.
			 */
			[[nodiscard]] constexpr const char* Name() const noexcept {
				return m_name;
			}

			/**
			 * @return Feature set.
			 */
			[[nodiscard]] constexpr const class Features& Features() const noexcept {
				return m_features;
			}

		private:
			const char* m_name;				///< Name
			class Features m_features;		///< Capabilities
	};
}
