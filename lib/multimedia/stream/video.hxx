#pragma once

#include <multimedia/codec/video/base.hxx>
#include <multimedia/property/hdr10.hxx>
#include <multimedia/property/resolution.hxx>
#include <multimedia/stream/base.hxx>

#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Video
	 * @brief The class for video streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the video.
			 * @param res The resolution of the video.
			 * @param lang The language of the video.
			 */
			Video(const Codec::Video::Base& codec, const Property::Resolution& res, const std::optional<Property::Language>& lang = std::nullopt);

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the video.
			 * @param res The resolution of the video.
			 * @param lang The language of the video.
			 */
			Video(Codec::Video::Base&& codec, Property::Resolution&& res, std::optional<Property::Language>&& lang) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Video(const Video& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Video(Video&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Video& operator=(const Video& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Video& operator=(Video&& stream) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			~Video() noexcept override 					= default;

			/**
			 * @brief Get the size of the video.
			 * @return The size of the video.
			 */
			const Property::Resolution& 				GetResolution() const noexcept;

			/**
			 * @brief Get the color primaries of the video.
			 * @return The color primaries of the video.
			 */
			const std::optional<Property::Color>&		GetColor() const noexcept;

			/**
			 * @brief Set the color primaries of the video.
			 * @param color The color primaries of the video.
			 */
			void 										SetColor(const Property::Color& color);

			/**
			 * @brief Set the color primaries of the video.
			 * @param color The color primaries of the video.
			 */
			void 										SetColor(Property::Color&& color) noexcept;

			/**
			 * @brief Get the HDR10 properties of the video.
			 * @return The HDR10 properties of the video.
			 */
			const std::optional<Property::HDR10>&		GetHDR10() const noexcept;
			
			/**
			 * @brief Set the HDR10 properties of the video.
			 * @param hdr10 The HDR10 properties of the video.
			 */
			void 										SetHDR10(const Property::HDR10& hdr10);

			/**
			 * @brief Set the HDR10 properties of the video.
			 * @param hdr10 The HDR10 properties of the video.
			 */
			void 										SetHDR10(Property::HDR10&& hdr10) noexcept;

			/**
			 * @brief Get the number of frames in the video.
			 * @return The number of frames in the video.
			 */
			const std::optional<unsigned int>&			GetFrames() const noexcept;

			/**
			 * @brief Set the number of frames in the video.
			 * @param frames The number of frames in the video.
			 */
			void 										SetFrames(unsigned int frames);

			/**
			 * @brief Clone the stream.
			 * @return The cloned stream.
			 */
			std::shared_ptr<Base> 						Clone() const override;

			/**
			 * @brief Move the stream.
			 * @return The moved stream.
			 */
			std::shared_ptr<Base> 						Move() noexcept override;

		private:
			Property::Resolution m_res; 				///< The resolution of the video.
			std::optional<Property::Color> m_color;		///< The color primaries of the video.
			std::optional<Property::HDR10> m_hdr10;		///< The HDR10 properties of the video.
			std::optional<unsigned int> m_frames;		///< The number of frames in the video.
	};
}