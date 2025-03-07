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
	class STORMBYTE_MULTIMEDIA_PUBLIC Base: public Util::Templates::Clonable<Base, std::unique_ptr<Base>> {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the Codec.
			 * @param type The type of the Codec.
			 */
			constexpr Base(const Codec::Name& name, const Property::Type& type) noexcept:
				m_name(name), m_type(type) {}

			/**
			 * @brief Default constructor.
			 * @param name The name of the Codec.
			 * @param type The type of the Codec.
			 */
			constexpr Base(Codec::Name&& name, const Property::Type& type) noexcept:
				m_name(std::move(name)), m_type(type) {}

			/**
			 * @brief Copy constructor.
			 * @param codec The Base to copy.
			 */
			constexpr Base(const Base& codec) 					= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Base to move.
			 */
			constexpr Base(Base&& codec) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The Base to copy.
			 * @return The copied Base.
			 */
			constexpr Base& operator=(const Base& codec) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The Base to move.
			 * @return The moved Base.
			 */
			constexpr Base& operator=(Base&& codec) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr virtual ~Base() noexcept					= default;

			/**
			 * @brief Get the codec name
			 * @return The codec name
			 */
			constexpr const Codec::Name&						GetName() const noexcept {
				return m_name;
			}

			/**
			 * @brief Get the type of the Base.
			 * @return The type of the Base.
			 */
			constexpr const Property::Type&						GetType() const noexcept {
				return m_type;
			}

		protected:
			Codec::Name m_name;									///< The name of the Base.
			Property::Type m_type;								///< The type of the Base.
	};
}