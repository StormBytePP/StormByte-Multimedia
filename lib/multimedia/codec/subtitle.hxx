#pragma once

#include <multimedia/codec/base.hxx>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	/**
	 * @class Subtitle
	 * @brief The class for all subtitle codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Subtitle: public Base {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Subtitle(const std::string& name);

			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Subtitle(std::string&& name) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			Subtitle(const Subtitle& codec) 						= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			Subtitle(Subtitle&& codec) noexcept						= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return The copied codec.
			 */
			Subtitle& operator=(const Subtitle& codec) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return The moved Subtitle.
			 */
			Subtitle& operator=(Subtitle&& codec) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Subtitle() noexcept override 				= default;
	};
}