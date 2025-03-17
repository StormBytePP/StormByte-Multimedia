#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media types.
 */
namespace StormByte::Multimedia::Media {
	class STORMBYTE_MULTIMEDIA_PUBLIC Sample {
		public:
			/**
			 * @brief Constructor.
			 * @param format The sample format
			 * @param rate The sample rate
			 */
			Sample(const std::string& format, const unsigned int& rate);

			/**
			 * @brief Copy constructor
			 * @param sample The Sample to copy.
			 */
			Sample(const Sample& sample)				= default;

			/**
			 * @brief Move constructor.
			 * @param sample The Sample to move.
			 */
			Sample(Sample&& sample) noexcept			= default;

			/**
			 * Copy assignment operator.
			 * @param sample The Sample to copy.
			 * @return The copied Sample.
			 */
			Sample& operator=(const Sample& sample)	= default;

			/**
			 * Move assignment operator.
			 * @param sample The Sample to move.
			 * @return The moved Sample.
			 */
			Sample& operator=(Sample&& sample) noexcept	= default;

			/**
			 * Gets format
			 * @return The format
			 */
			std::string& Format() noexcept;

			/**
			 * Gets format
			 * @return The format
			 */
			const std::string& Format() const noexcept;

			/**
			 * Gets rate
			 * @return The rate
			 */
			unsigned int& Rate() noexcept;

			/**
			 * Gets rate
			 * @return The rate
			 */
			const unsigned int& Rate() const noexcept;

		private:
			std::string m_format;		///< The format.
			unsigned int m_rate;		///< The size.
	};
}