#pragma once

#include <multimedia/exception.hxx>
#include <multimedia/media/codec.hxx>
#include <multimedia/media/language.hxx>
#include <util/templates/clonable.hxx>

#include <optional>

namespace StormByte::Multimedia {
	// Forward declarations
	class AudioStream;
	class VideoStream;
	class SubtitleStream;

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
			constexpr ~Stream() noexcept 								= default;

			/**
			 * @brief Gets the codec of the stream.
			 * @return The codec of the stream.
			 */
			constexpr Media::Codec 										GetCodec() const noexcept {
				return m_codec;
			}

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			constexpr Media::Type 										GetType() const noexcept {
				return Media::CodecType(m_codec);
			}

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
			template<Media::Codec C>
			static PointerType 											Create() {
				switch (Media::CodecType(C)) {
					case Media::Type::Audio:
						return MakePointer<AudioStream>(C);
					case Media::Type::Video:
						return MakePointer<VideoStream>(C);
					case Media::Type::Subtitle:
						return MakePointer<SubtitleStream>(C);
					case Media::Type::Image:
						return MakePointer<SubtitleStream>(C);
					default:
						return nullptr;
				}
			}

		protected:
			Media::Codec m_codec;										///< The codec of the stream.
			std::optional<Media::Language> m_language;					///< The language of the stream.

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr Stream(const Media::Codec& codec) noexcept {
				m_codec = codec;
			}
    };

	/**
	 * @class AudioStream
	 * @brief Class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC AudioStream: public Stream {
		friend class Stream;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr AudioStream(const AudioStream& stream) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr AudioStream(AudioStream&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr AudioStream& operator=(const AudioStream& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr AudioStream& operator=(AudioStream&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~AudioStream() noexcept 										= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 															Clone() const override {
				return MakePointer<AudioStream>(*this);
			}

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 															Move() override {
				return MakePointer<AudioStream>(std::move(*this));
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr AudioStream(const Media::Codec& codec) noexcept: Stream(codec) {}
	};

	/**
	 * @class VideoStream
	 * @brief Class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC VideoStream: public Stream {
		friend class Stream;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr VideoStream(const VideoStream& stream) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr VideoStream(VideoStream&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr VideoStream& operator=(const VideoStream& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr VideoStream& operator=(VideoStream&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~VideoStream() noexcept 										= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 															Clone() const override {
				return MakePointer<VideoStream>(*this);
			}

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 															Move() override {
				return MakePointer<VideoStream>(std::move(*this));
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr VideoStream(const Media::Codec& codec) noexcept: Stream(codec) {}
	};

	/**
	 * @class SubtitleStream
	 * @brief Class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC SubtitleStream: public Stream {
		friend class Stream;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr SubtitleStream(const SubtitleStream& stream) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr SubtitleStream(SubtitleStream&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr SubtitleStream& operator=(const SubtitleStream& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr SubtitleStream& operator=(SubtitleStream&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~SubtitleStream() noexcept 										= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 															Clone() const override {
				return MakePointer<SubtitleStream>(*this);
			}

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 															Move() override {
				return MakePointer<SubtitleStream>(std::move(*this));
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr SubtitleStream(const Media::Codec& codec) noexcept: Stream(codec) {}
	};

	/**
	 * @class ImageStream
	 * @brief Class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ImageStream: public Stream {
		friend class Stream;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr ImageStream(const ImageStream& stream) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr ImageStream(ImageStream&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr ImageStream& operator=(const ImageStream& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr ImageStream& operator=(ImageStream&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~ImageStream() noexcept 										= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 															Clone() const override {
				return MakePointer<ImageStream>(*this);
			}

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 															Move() override {
				return MakePointer<ImageStream>(std::move(*this));
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr ImageStream(const Media::Codec& codec) noexcept: Stream(codec) {}
	};
}