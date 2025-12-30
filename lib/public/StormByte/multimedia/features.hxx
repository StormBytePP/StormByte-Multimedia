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
	 * @brief Wrapper class for handling Feature bitmasks.
	 *
	 * Provides ergonomic addition, removal and checking of features.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Features: public StormByte::Bitmask<Features, Feature> {
		public:
			/**
			 * @brief Default constructor.
			 */
			constexpr Features() noexcept: StormByte::Bitmask<Features, Feature>() {}

			/**
			 * @brief Constructor with initial feature.
			 * @param feature The initial feature to set.
			 */
			constexpr Features(Feature feature) noexcept
			: StormByte::Bitmask<Features, Feature>(feature) {}

			/**
			 * @brief Copy constructor.
			 * @param other The other Features to copy from.
			 */
			constexpr Features(const Features& other) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param other The other Features to move from.
			 */
			constexpr Features(Features&& other) noexcept 					= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr ~Features() noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other Features to copy from.
			 * @return Features& Reference to this Features.
			 */
			constexpr Features& operator=(const Features& other) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other Features to move from.
			 * @return Features& Reference to this Features.
			 */
			constexpr Features& operator=(Features&& other) noexcept 		= default;

			/**
			 * @brief Equality operator.
			 * @param other The other Features to compare with.
			 * @return true if equal, false otherwise.
			 */
			[[nodiscard]]
			constexpr bool operator==(const Features& other) const noexcept {
				return Bitmask<Features, Feature>::operator==(other);
			}

			/**
			 * @brief Inequality operator.
			 * @param other The other Features to compare with.
			 * @return true if not equal, false otherwise.
			 */
			[[nodiscard]]
			constexpr bool operator!=(const Features& other) const noexcept {
				return Bitmask<Features, Feature>::operator!=(other);
			}

			operator std::string() const noexcept;
	};
}