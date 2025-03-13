#pragma once

#include <multimedia/stream/base.hxx>

/**
 * @namespace Stream
 * @brief The namespace for all multimedia streams types.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Audio
	 * @brief Class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final: public Base {
		friend class Base;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			constexpr Audio(const Audio& stream) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			constexpr Audio(Audio&& stream) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			constexpr Audio& operator=(const Audio& stream) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			constexpr Audio& operator=(Audio&& stream) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~Audio() noexcept 								= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 												Clone() const override;

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 												Move() override;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			constexpr Media::Type 										Type() const noexcept {
				return Media::Type::Audio;
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			constexpr Audio(const Media::Codec::Name& codec) noexcept: Base(codec) {}
	};
}