#pragma once

#include <multimedia/exception.hxx>
#include <multimedia/media/codec.hxx>
#include <multimedia/media/language.hxx>
#include <util/templates/clonable.hxx>

#include <optional>
#include <span>

/**
 * @namespace Stream
 * @brief The namespace for all multimedia streams types.
 */
namespace StormByte::Multimedia::Stream {
	// Forward declarations
	class Audio;
	class Video;
	class Subtitle;
	class Image;
	class Attachment;

	/**
	 * @class Base
	 * @brief Base class for all multimedia streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Base: public Util::Templates::Clonable<Base, std::shared_ptr<Base>> {
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr Base(const Base& stream) noexcept 			= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr Base(Base&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr Base& operator=(const Base& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr Base& operator=(Base&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr virtual ~Base() noexcept 						= default;

			/**
			 * @brief Gets the codec of the stream.
			 * @return The codec of the stream.
			 */
			constexpr Media::Codec::Name							Codec() const noexcept {
				return m_codec;
			}

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			virtual Media::Type 									Type() const noexcept = 0;

			/**
			 * @brief Gets the language of the stream.
			 * @return The language of the stream.
			 */
			constexpr std::optional<Media::Language> 				Language() const noexcept {
				return m_language;
			}

			/**
			 * @brief Sets the language of the stream.
			 * @param language The language of the stream.
			 */
			constexpr void 											Language(const Media::Language& language) noexcept {
				m_language = language;
			}

			/**
			 * @brief Creates a new stream of the corresponding type based on the provided codec
			 * @param codec Codec for the stream
			 * @return The created stream.
			 */
			static PointerType 										Create(const Media::Codec::Name& codec);

		protected:
			Media::Codec::Name m_codec;								///< The codec of the stream.
			std::optional<Media::Language> m_language;				///< The language of the stream.

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr Base(const Media::Codec::Name& codec) noexcept {
				m_codec = codec;
			}
    };
	using PointerType	= Base::PointerType;						///< PointerType alias
	using Span			= std::span<PointerType>;					///< Span type
	using ConstSpan		= std::span<const PointerType>;				///< Const span type 
}
