#pragma once

#include <multimedia/codec/codec.hxx>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	/**
	 * @class Video
	 * @brief The class for all video codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Codec {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Video(const std::string& name);

			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Video(std::string&& name) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			Video(const Video& codec) 						= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			Video(Video&& codec) noexcept					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return The copied codec.
			 */
			Video& operator=(const Video& codec) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return The moved Video.
			 */
			Video& operator=(Video&& codec) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			~Video() noexcept override 						= default;
	};
}