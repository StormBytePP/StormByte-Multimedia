#pragma once

#include <multimedia/codec/codec.hxx>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	/**
	 * @class Image
	 * @brief The class for all image codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Image final: public Codec {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Image(const std::string& name);

			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Image(std::string&& name) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			Image(const Image& codec) 						= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			Image(Image&& codec) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return The copied codec.
			 */
			Image& operator=(const Image& codec) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return The moved Image.
			 */
			Image& operator=(Image&& codec) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			~Image() noexcept override						= default;

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