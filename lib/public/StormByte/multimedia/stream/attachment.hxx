#pragma once

#include <StormByte/multimedia/stream/base.hxx>

/**
 * @namespace Stream
 * @brief The namespace for all multimedia streams types.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Attachment
	 * @brief Class for attachment streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Attachment final: public Base {
		friend class Base;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Attachment(const Attachment& stream) noexcept 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Attachment(Attachment&& stream) noexcept							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			Attachment& operator=(const Attachment& stream) noexcept 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			Attachment& operator=(Attachment&& stream) noexcept 				= default;

			/**
			 * @brief Destructor.
			 */
			~Attachment() noexcept override 									= default;

			/**
			 * @brief Clones the stream.
			 * @return The cloned stream.
			 */
			PointerType 														Clone() const override;

			/**
			 * @brief Moves the stream.
			 * @return The moved stream.
			 */
			PointerType 														Move() override;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			constexpr Media::Type 												Type() const noexcept {
				return Media::Type::Attachment;
			}

		private:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			Attachment(const Media::Codec::Name& codec) noexcept: Base(codec) {}
	};
}