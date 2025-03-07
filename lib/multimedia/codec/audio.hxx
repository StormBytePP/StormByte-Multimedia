#pragma once

#include <multimedia/codec/codec.hxx>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	/**
	 * @class Audio
	 * @brief The class for all audio codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final: public Codec {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Audio(const std::string& name);

			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Audio(std::string&& name) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			Audio(const Audio& codec) 						= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			Audio(Audio&& codec) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return The copied codec.
			 */
			Audio& operator=(const Audio& codec) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return The moved Audio.
			 */
			Audio& operator=(Audio&& codec) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			~Audio() noexcept override 						= default;

			/**
			 * @brief Clones the codec.
			 * @return The cloned codec.
			 */
			std::shared_ptr<Codec> 							Clone() const override;

			/**
			 * @brief Moves the codec.
			 * @return The moved codec.
			 */
			std::shared_ptr<Codec> 							Move() noexcept override;
	};
}