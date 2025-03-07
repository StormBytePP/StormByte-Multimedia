#pragma once

#include <multimedia/codec/image.hxx>
#include <multimedia/property/resolution.hxx>
#include <multimedia/stream/base.hxx>

#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Image
	 * @brief The class for image streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Image final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the image.
			 * @param res The resolution of the image.
			 */
			Image(const Codec::Image& codec, const Property::Resolution& res);

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the image.
			 * @param res The resolution of the image.
			 */
			Image(Codec::Image&& codec, Property::Resolution&& res) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Image(const Image& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Image(Image&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Image& operator=(const Image& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Image& operator=(Image&& stream) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			~Image() noexcept override 					= default;

			/**
			 * @brief Get the size of the image.
			 * @return The size of the image.
			 */
			const Property::Resolution& 				GetResolution() const noexcept;

			/**
			 * Clone the stream.
			 * @return The cloned stream.
			 */
			std::shared_ptr<Base> Clone() const override;

			/**
			 * Move the stream.
			 * @return The moved stream.
			 */
			std::shared_ptr<Base> Move() noexcept override;

		private:
			Property::Resolution m_res; 				///< The resolution of the image.
	};
}