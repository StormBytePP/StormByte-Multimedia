#pragma once

#include <multimedia/codec/base.hxx>

/**
 * @namespace Audio
 * @brief The namespace for all audio codecs.
 */
namespace StormByte::Multimedia::Codec::Audio {
	/**
	 * @class Base
	 * @brief The class for all audio codecs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Base: public Codec::Base {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Base(const Name& name);

			/**
			 * @brief Default constructor.
			 * @param name The name of the codec.
			 */
			Base(Name&& name) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			Base(const Base& codec) 						= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			Base(Base&& codec) noexcept						= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return The copied codec.
			 */
			Base& operator=(const Base& codec) 				= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return The moved Audio.
			 */
			Base& operator=(Base&& codec) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Base() noexcept override 				= default;
	};
}