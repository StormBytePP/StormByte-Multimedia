#pragma once

#include <util/templates/clonable.hxx>
#include <multimedia/codec/name.hxx>
#include <multimedia/property/type.hxx>

#include <string>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	/**
	 * @class Base
	 * @brief The base class for all codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Base: public Util::Templates::Clonable<Base> {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the Codec.
			 * @param type The type of the Codec.
			 */
			Base(const Codec::Name& name, const Property::Type& type);

			/**
			 * @brief Default constructor.
			 * @param name The name of the Codec.
			 * @param type The type of the Codec.
			 */
			Base(Codec::Name&& name, const Property::Type& type) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Base to copy.
			 */
			Base(const Base& codec) 					= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Base to move.
			 */
			Base(Base&& codec) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The Base to copy.
			 * @return The copied Base.
			 */
			Base& operator=(const Base& codec) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The Base to move.
			 * @return The moved Base.
			 */
			Base& operator=(Base&& codec) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Base() noexcept					= 0;

			/**
			 * @brief Get the codec name
			 * @return The codec name
			 */
			const Codec::Name&							GetName() const noexcept;

			/**
			 * @brief Get the type of the Base.
			 * @return The type of the Base.
			 */
			const Property::Type&						GetType() const noexcept;

		protected:
			Codec::Name m_name;							///< The name of the Base.
			Property::Type m_type;						///< The type of the Base.
	};
}