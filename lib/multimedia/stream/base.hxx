#pragma once

#include <util/templates/clonable.hxx>
#include <multimedia/codec/base.hxx>
#include <multimedia/property/language.hxx>

#include <optional>
#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Base
	 * @brief The base class for streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Base: public Util::Templates::Clonable<Base> {
		public:
			/**
			 * @brief Default constructor.
			 * @param type The type of the Stream.
			 * @param codec The codec of the Stream.
			 * @param lang The language of the Stream.
			 */
			Base(const Property::Type& type, const Codec::Base& codec, const std::optional<Property::Language>& lang = std::nullopt);

			/**
			 * @brief Default constructor.
			 * @param type The type of the Stream.
			 * @param codec The codec of the Stream.
			 * @param lang The language of the Stream.
			 */
			Base(const Property::Type& type, Codec::Base&& codec, std::optional<Property::Language>&& lang) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Base(const Base& stream) 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Base(Base&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Base& operator=(const Base& stream) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Base& operator=(Base&& stream) noexcept			= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Base() noexcept						= 0;

			/**
			 * @brief Get the type of the Codec.
			 * @return The type of the Codec.
			 */
			const Property::Type& GetType() const noexcept;

			/**
			 * @brief Get the language of the Stream.
			 * @return The language of the Stream.
			 */
			const std::optional<Property::Language>& 		GetLanguage() const noexcept;

			/**
			 * @brief Set the language of the Stream.
			 * @param lang The language of the Stream.
			 */
			void 											SetLanguage(const Property::Language& lang) noexcept;

			/**
			 * @brief Set the language of the Stream.
			 * @param lang The language of the Stream.
			 */
			void 											SetLanguage(Property::Language&& lang) noexcept;

			/**
			 * @brief Get the codec of the Stream.
			 * @return The codec of the Stream.
			 */
			std::shared_ptr<Codec::Base> 					GetCodec() const noexcept;

		protected:
			Property::Type m_type; 							///< The type of the Codec.
			std::shared_ptr<Codec::Base> m_codec;			///< The codec of the Stream.
			std::optional<Property::Language> m_lang;		///< The language of the Stream.
	};
}