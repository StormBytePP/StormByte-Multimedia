#pragma once

#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/context/generic.hxx>
#include <StormByte/multimedia/metadata.hxx>
#include <StormByte/multimedia/type.hxx>

#include <memory>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Stream
	 * @brief The class representing a multimedia stream.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @brief Constructor.
			 * @param codec The codec of the stream.
			 * @param type The type of the stream.
			 */
			Stream(const Codec& codec, enum Type type) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other The other stream to copy from.
			 */
			Stream(const Stream& other)								= default;

			/**
			 * @brief Move constructor.
			 * @param other The other stream to move from.
			 */
			Stream(Stream&& other) noexcept							= default;

			/**
			 * @brief Default destructor.
			 */
			~Stream() noexcept 										= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other stream to copy from.
			 * @return Reference to this stream.
			 */
			Stream& operator=(const Stream& other)					= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other stream to move from.
			 * @return Reference to this stream.
			 */
			Stream& operator=(Stream&& other)						= default;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			enum Type												Type() const noexcept;

			/**
			 * @brief Gets the metadata of the stream.
			 * @return The metadata of the stream.
			 */
			const class Metadata&									Metadata() const noexcept;

			/**
			 * @brief Sets the metadata of the stream.
			 * @param metadata The metadata of the stream.
			 */
			void													Metadata(class Metadata&& metadata) noexcept;

			/**
			 * @brief Gets the context of the stream.
			 * @return The context of the stream.
			 */
			std::shared_ptr<const Context::Generic>					Context() const noexcept;

			/**
			 * @brief Sets the context of the stream.
			 * @param context The context of the stream.
			 */
			void													Context(Context::Generic&& context) noexcept;

			/**
			 * @brief Gets the codec of the stream.
			 * @return The codec of the stream.
			 */
			const class Codec&										Codec() const noexcept;

		private:
			class Codec m_codec;									///< The codec of the stream
			enum Type m_type;										///< The type of the stream
			class Metadata m_metadata;								///< The metadata of the stream
			std::shared_ptr<Context::Generic> m_context;			///< The context of the stream
	};
}