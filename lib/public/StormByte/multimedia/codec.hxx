#pragma once

#include <StormByte/multimedia/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Codec
	 * @brief The class representing a multimedia codec.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Codec {
		public:
			/**
			 * @brief Copy constructor.
			 * @param other The other codec to copy from.
			 */
			Codec(const Codec& other)								= default;

			/**
			 * @brief Move constructor.
			 * @param other The other codec to move from.
			 */
			Codec(Codec&& other) noexcept							= default;

			/**
			 * @brief Default destructor.
			 */
			~Codec() noexcept 										= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other codec to copy from.
			 * @return Reference to this codec.
			 */
			Codec& operator=(const Codec& other)					= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other codec to move from.
			 * @return Reference to this codec.
			 */
			Codec& operator=(Codec&& other)							= default;

			/**
			 * @brief Gets the name of the codec.
			 * @return The name of the codec.
			 */
			std::string 											Name() const noexcept;

			/**
			 * @brief Gets the description of the codec.
			 * @return The description of the codec.
			 */
			std::string 											Description() const noexcept;

			/**
			 * @brief Finds a codec by name.
			 * @param name The name of the codec.
			 * @return ExpectedCodec containing the found codec or CodecNotFound exception.
			 */
			static ExpectedCodec 									Find(const std::string& name) noexcept;

			/**
			 * @brief Finds a codec by ID.
			 * @param id The ID of the codec.
			 * @return ExpectedCodec containing the found codec or CodecNotFound exception.
			 */
			static ExpectedCodec 									Find(int id) noexcept;

			/**
			 * @brief Checks if the codec has an encoder.
			 * @return True if the codec has an encoder, false otherwise.
			 */
			bool 													HasDecoder() const noexcept;

			/**
			 * @brief Checks if the codec has a decoder.
			 * @return True if the codec has a decoder, false otherwise.
			 */
			bool 													HasEncoder() const noexcept;

		private:
			int m_codec_id;											///< The Codec ID
			std::string m_name, m_description;						///< The name and description of the codec

			/**
			 * @brief Private constructor from AVCodec pointer.
			 * @param codec_id The Codec ID
			 * @param name The name of the codec
			 * @param description The description of the codec
			 */
			Codec(int codec_id, const std::string& name, const std::string& description) noexcept;
	};
}