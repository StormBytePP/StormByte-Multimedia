#pragma once

#include <multimedia/codec/audio/base.hxx>

/**
 * @namespace Audio
 * @brief The namespace for all audio codecs.
 */
namespace StormByte::Multimedia::Codec::Audio {
	/**
	 * @class Base
	 * @brief The class for all audio codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC EAC3 final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			constexpr EAC3() noexcept:
				Base(Name::EAC3) {}

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			constexpr EAC3(const EAC3& codec) 					= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			constexpr EAC3(EAC3&& codec) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return The copied codec.
			 */
			constexpr EAC3& operator=(const EAC3& codec) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return The moved Audio.
			 */
			constexpr EAC3& operator=(EAC3&& codec) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr ~EAC3() noexcept override 				= default;

			/**
			 * @brief Clones the codec.
			 * @return The cloned codec.
			 */
			PointerType 										Clone() const override;

			/**
			 * @brief Moves the codec.
			 * @return The moved codec.
			 */
			PointerType 										Move() noexcept override;
	};
}