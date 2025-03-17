#pragma once

#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/codec.hxx>
#include <StormByte/multimedia/media/tags.hxx>
#include <StormByte/util/clonable.hxx>


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
	class STORMBYTE_MULTIMEDIA_PUBLIC Base: public Util::Clonable<Base, std::shared_ptr<Base>> {
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Base(const Base& stream) noexcept 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Base(Base&& stream) noexcept							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			Base& operator=(const Base& stream) noexcept 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			Base& operator=(Base&& stream) noexcept 				= default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Base() noexcept override 						= default;

			/**
			 * @brief Creates a new stream of the corresponding type based on the provided codec
			 * @param codec Codec for the stream
			 * @return The created stream.
			 */
			static PointerType 										Create(const Media::Codec::ID& codec);

			/**
			 * @brief Gets the codec of the stream.
			 * @return The codec of the stream.
			 */
			constexpr Media::Codec::ID								Codec() const noexcept {
				return m_codec;
			}

			/**
			 * @brief Gets the disposition of the stream.
			 * @return The disposition of the stream.
			 */
			Media::Tags& 											Disposition() noexcept;

			/**
			 * @brief Gets the disposition of the stream.
			 * @return The disposition of the stream.
			 */
			const Media::Tags& 										Disposition() const noexcept;

			/**
			 * @brief Gets the tags of the stream.
			 * @return The tags of the stream.
			 */
			Media::Tags& 											Tags() noexcept;

			/**
			 * @brief Gets the tags of the stream.
			 * @return The tags of the stream.
			 */
			const Media::Tags& 										Tags() const noexcept;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			virtual Media::Type 									Type() const noexcept = 0;

		protected:
			Media::Codec::ID m_codec;								///< The codec of the stream.
			Media::Tags m_disposition;								///< The disposition of the stream.
			Media::Tags m_tags;										///< The tags of the stream.

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			Base(const Media::Codec::ID& codec) noexcept;
    };
	using PointerType	= Base::PointerType;						///< PointerType alias
	using Span			= std::span<PointerType>;					///< Span type
	using ConstSpan		= std::span<const PointerType>;				///< Const span type

	STORMBYTE_MULTIMEDIA_PUBLIC PointerType Create(const Media::Codec::ID& codec);
}
