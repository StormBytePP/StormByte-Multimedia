#pragma once

#include <StormByte/multimedia/engine/codec.hxx>
#include <StormByte/multimedia/context/audio.hxx>
#include <StormByte/multimedia/context/video.hxx>
#include <StormByte/multimedia/metadata.hxx>
#include <StormByte/multimedia/type.hxx>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Stream
	 * @brief One media stream: codec, type, metadata and optional context.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @param codec Stream codec.
			 * @param type Stream type (may be adjusted for mjpeg → Attachment).
			 */
			Stream(const Codec& codec, enum Type type) noexcept;

			/**
			 * Copy constructor.
			 */
			Stream(const Stream& other) = default;

			/**
			 * Move constructor.
			 */
			Stream(Stream&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Stream() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Stream& operator=(const Stream& other) = default;

			/**
			 * Move assignment.
			 */
			Stream& operator=(Stream&& other) = default;

			/**
			 * @return Stream type.
			 */
			enum Type Type() const noexcept;

			/**
			 * @return Stream metadata tags.
			 */
			const StormByte::Multimedia::Metadata& Metadata() const noexcept;

			/**
			 * Replaces stream metadata.
			 * @param metadata New tags.
			 */
			void Metadata(class Metadata&& metadata) noexcept;

			/**
			 * @return Optional typed context (audio/video), or null.
			 */
			std::shared_ptr<const Context::Generic> Context() const noexcept;

			/**
			 * Sets stream context (takes ownership via Move()).
			 * @param context Context object.
			 */
			void Context(Context::Generic&& context) noexcept;

			/**
			 * @return Stream codec.
			 */
			const class Codec& Codec() const noexcept;

		private:
			class Codec m_codec;									///< Codec
			enum Type m_type;										///< Type
			class Metadata m_metadata;								///< Tags
			std::shared_ptr<Context::Generic> m_context;			///< Optional context
	};
}
