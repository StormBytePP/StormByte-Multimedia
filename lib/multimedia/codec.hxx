#pragma once

#include <multimedia/media/type.hxx>
#include <util/templates/clonable.hxx>

namespace StormByte::Multimedia {
    /**
     * @class Codec
     * @brief The template class for all multimedia codecs.
     */
    template <Media::Codec C>
	class STORMBYTE_MULTIMEDIA_PUBLIC Codec: public Util::Templates::Clonable<Codec<C>> {
		public:
			/**
			 * @brief Default constructor.
			 */
			constexpr Codec() noexcept 									= default;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			constexpr Codec(const Codec& codec) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			constexpr Codec(Codec&& codec) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The Codec to copy.
			 * @return The copied Codec.
			 */
			constexpr Codec& operator=(const Codec& codec) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The Codec to move.
			 * @return The moved Codec.
			 */
			constexpr Codec& operator=(Codec&& codec) noexcept 			= default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~Codec() noexcept override 						= default;

			/**
			 * @brief Gets the name of the codec.
			 * @return The codec name.
			 */
			constexpr Media::Codec 										GetName() const noexcept {
				return C;
			}

			/**
			 * @brief Gets the type of the codec.
			 * @return The codec type.
			 */
			constexpr Media::Type 										GetType() const noexcept {
				return Media::CodecType<C>;
			}

			// /**
			//  * @brief Clones the codec.
			//  * @return The cloned codec.
			//  */
			// PointerType 												Clone() const override {
			// 	return MakePointer<Codec<C>>(*this);
			// }

			// /**
			//  * @brief Moves the codec.
			//  * @return The moved codec.
			//  */
			// PointerType 												Move() override {
			// 	return MakePointer<Codec<C>>(std::move(*this));
			// }
	};
}