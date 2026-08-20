#pragma once

#include <StormByte/bitmask.hxx>
#include <StormByte/multimedia/feature.hxx>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Features
	 * @brief Bitmask of Feature flags with ergonomic helpers.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Features: public StormByte::Bitmask<Features, Feature> {
		public:
			/**
			 * Default constructor (empty set).
			 */
			constexpr Features() noexcept: StormByte::Bitmask<Features, Feature>() {}

			/**
			 * @param feature Initial feature.
			 */
			constexpr Features(Feature feature) noexcept
			: StormByte::Bitmask<Features, Feature>(feature) {}

			/**
			 * Copy constructor.
			 */
			constexpr Features(const Features& other) noexcept = default;

			/**
			 * Move constructor.
			 */
			constexpr Features(Features&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			constexpr ~Features() noexcept = default;

			/**
			 * Copy assignment.
			 */
			constexpr Features& operator=(const Features& other) noexcept = default;

			/**
			 * Move assignment.
			 */
			constexpr Features& operator=(Features&& other) noexcept = default;

			/**
			 * Equality.
			 * @param other Other set.
			 * @return true if equal.
			 */
			[[nodiscard]]
			constexpr bool operator==(const Features& other) const noexcept {
				return Bitmask<Features, Feature>::operator==(other);
			}

			/**
			 * Inequality.
			 * @param other Other set.
			 * @return true if not equal.
			 */
			[[nodiscard]]
			constexpr bool operator!=(const Features& other) const noexcept {
				return Bitmask<Features, Feature>::operator!=(other);
			}

			/**
			 * Human-readable list of enabled features ("A | B | C").
			 */
			operator std::string() const noexcept;
	};
}
