#pragma once

#include <multimedia/codec/subtitle/base.hxx>
#include <multimedia/stream/base.hxx>

#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Subtitle
	 * @brief The class for subtitle streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Subtitle final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the subtitle.
			 * @param lang The language of the subtitle.
			 */
			Subtitle(const Codec::Subtitle::Base& codec, const std::optional<Property::Language>& lang = std::nullopt);

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the subtitle.
			 * @param lang The language of the subtitle.
			 */
			Subtitle(Codec::Subtitle::Base&& codec, Property::Language&& lang) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Subtitle(const Subtitle& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Subtitle(Subtitle&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Subtitle& operator=(const Subtitle& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Subtitle& operator=(Subtitle&& stream) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			~Subtitle() noexcept override 						= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 								Clone() const override;

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 								Move() noexcept override;
	};
}