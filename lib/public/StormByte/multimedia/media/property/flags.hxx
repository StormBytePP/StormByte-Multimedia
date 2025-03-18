#pragma once

#include <StormByte/multimedia/visibility.h>
#include <StormByte/util/clonable.hxx>

#include <string>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Media::Property {
	/**
	 * @brief Flags class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Flags: public StormByte::Util::Clonable<Flags, std::shared_ptr<Flags>> {
		public:
			/**
			 * @brief Constructor.
			 * @param flags The flags.
			 */
			Flags(const std::string& flags);

			/**
			 * @brief Copy constructor.
			 * @param other The Flags object to copy.
			 */
			Flags(const Flags& other)								= default;

			/**
			 * @brief Move constructor.
			 * @param other The Flags object to move.
			 */
			Flags(Flags&& other) noexcept							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The Flags object to copy.
			 * @return A reference to this object.
			 */
			Flags& operator=(const Flags& other)					= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The Flags object to move.
			 * @return A reference to this object.
			 */
			Flags& operator=(Flags&& other) noexcept				= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Flags() noexcept 								= default;

			/**
			 * @brief Gets the flags as string.
			 * @return The flags.
			 */
			std::string 											ToString() const;

		protected:
			const std::string 										m_flags;		///< The flags.
	};	
}