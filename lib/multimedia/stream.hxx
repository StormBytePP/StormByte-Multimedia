#pragma once

#include <multimedia/exception.hxx>
#include <multimedia/media/codec.hxx>
#include <multimedia/media/language.hxx>
#include <util/templates/clonable.hxx>

#include <optional>

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
	 * @class Stream
	 * @brief Base class for all multimedia streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream: public Util::Templates::Clonable<Stream, std::shared_ptr<Stream>> {
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr Stream(const Stream& stream) noexcept 			= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr Stream(Stream&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr Stream& operator=(const Stream& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr Stream& operator=(Stream&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr virtual ~Stream() noexcept 						= default;

			/**
			 * @brief Gets the codec of the stream.
			 * @return The codec of the stream.
			 */
			constexpr Media::Codec::Name								GetCodec() const noexcept {
				return m_codec;
			}

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			virtual Media::Type 										GetType() const noexcept = 0;

			/**
			 * @brief Gets the language of the stream.
			 * @return The language of the stream.
			 */
			constexpr std::optional<Media::Language> 					GetLanguage() const noexcept {
				return m_language;
			}

			/**
			 * @brief Sets the language of the stream.
			 * @param language The language of the stream.
			 */
			constexpr void 												SetLanguage(const Media::Language& language) noexcept {
				m_language = language;
			}

			/**
			 * @brief Creates a new stream of the corresponding type based on the template parameter.
			 * @return The created audio stream.
			 */
			template<Media::Codec::Name C>
			static PointerType 											Create() {
				switch (Media::Codec::Registry::Info(C).s_type) {
					case Media::Type::Audio:
						return MakePointer<Audio>(C);
					case Media::Type::Video:
						return MakePointer<Video>(C);
					case Media::Type::Subtitle:
						return MakePointer<Subtitle>(C);
					case Media::Type::Image:
						return MakePointer<Image>(C);
					case Media::Type::Attachment:
						return MakePointer<Attachment>(C);
					default:
						return nullptr;
				}
			}

		protected:
			Media::Codec::Name m_codec;									///< The codec of the stream.
			std::optional<Media::Language> m_language;					///< The language of the stream.

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr Stream(const Media::Codec::Name& codec) noexcept {
				m_codec = codec;
			}
    };
}

/**
 * @note Include the corresponding stream type headers in order not to have compiler error if consumers forget to include some of them
 */
#include <multimedia/stream/audio.hxx>
#include <multimedia/stream/video.hxx>
#include <multimedia/stream/subtitle.hxx>
#include <multimedia/stream/image.hxx>
#include <multimedia/stream/attachment.hxx>