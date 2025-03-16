#pragma once

#include <StormByte/multimedia/stream/base.hxx>

/**
 * @namespace Stream
 * @brief The namespace for all multimedia streams types.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Subtitle
	 * @brief Class for subtitle streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Subtitle final: public Base {
		friend class Base;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Subtitle(const Subtitle& stream) noexcept 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Subtitle(Subtitle&& stream) noexcept							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			Subtitle& operator=(const Subtitle& stream) noexcept 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			Subtitle& operator=(Subtitle&& stream) noexcept 				= default;

			/**
			 * @brief Destructor.
			 */
			~Subtitle() noexcept override 									= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 													Clone() const override;

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 													Move() override;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			constexpr Media::Type 											Type() const noexcept {
				return Media::Type::Subtitle;
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			Subtitle(const Media::Codec::Name& codec) noexcept: Base(codec) {}
	};
}