#pragma once

#include <StormByte/multimedia/media/property/video/color.hxx>
#include <StormByte/multimedia/media/property/video/hdr10.hxx>
#include <StormByte/multimedia/media/property/resolution.hxx>
#include <StormByte/multimedia/stream/base.hxx>

#include <memory>

/**
 * @namespace Stream
 * @brief The namespace for all multimedia streams types.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Video
	 * @brief Class for video streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Base {
		friend class Base;
		public:
			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Video(const Video& stream) noexcept 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Video(Video&& stream) noexcept								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return Reference to the assigned stream.
			 */
			Video& operator=(const Video& stream) noexcept 				= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return Reference to the assigned stream.
			 */
			Video& operator=(Video&& stream) noexcept 					= default;

			/**
			 * @brief Destructor.
			 */
			~Video() noexcept override 									= default;

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
				return Media::Type::Video;
			}

			/**
			 * @brief Gets the color properties of the stream.
			 * @return The color properties of the stream.
			 */
			std::shared_ptr<const Media::Property::Video::Color>&		Color() noexcept;

			/**
			 * @brief Gets the color properties of the stream.
			 * @return The color properties of the stream.
			 */
			const std::shared_ptr<const Media::Property::Video::Color>&	Color() const noexcept;

			/**
			 * @brief Gets the resolution of the stream.
			 * @return The resolution of the stream.
			 */
			std::shared_ptr<const Media::Property::Resolution>&			Resolution() noexcept;

			/**
			 * @brief Gets the resolution of the stream.
			 * @return The resolution of the stream.
			 */
			const std::shared_ptr<const Media::Property::Resolution>&	Resolution() const noexcept;

			/**
			 * @brief Gets the profile of the stream.
			 * @return The profile of the stream.
			 */
			std::shared_ptr<const std::string>&							Profile() noexcept;

			/**
			 * @brief Gets the profile of the stream.
			 * @return The profile of the stream.
			 */
			const std::shared_ptr<const std::string>&					Profile() const noexcept;

		private:
			std::shared_ptr<const Media::Property::Video::Color>		m_color;		///< The color properties of the stream.
			std::shared_ptr<const Media::Property::Resolution>			m_resolution;	///< The resolution of the stream.
			std::shared_ptr<const std::string>							m_profile;		///< The profile of the stream.
			
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the stream.
			 */
			Video(std::shared_ptr<const Multimedia::Codec> codec) noexcept;
	};
}