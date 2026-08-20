#pragma once

#include <StormByte/multimedia/context/generic.hxx>
#include <StormByte/multimedia/context/property/color.hxx>
#include <StormByte/multimedia/context/property/hdr10.hxx>
#include <StormByte/multimedia/context/property/resolution.hxx>

#include <optional>

/**
 * @namespace Context
 * @brief Media stream context types (audio, video, …).
 */
namespace StormByte::Multimedia::Context {
	/**
	 * @class Video
	 * @brief Video stream context (color, resolution, optional HDR10).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Generic {
		public:
			/**
			 * @param color Color properties.
			 * @param resolution Frame size.
			 * @param hdr10 Optional HDR10 metadata (filled with DEFAULT if color allows HDR10 and none given).
			 */
			Video(Property::Color&& color, Property::Resolution&& resolution,
				std::optional<Property::HDR10>&& hdr10) noexcept;

			/**
			 * Copy constructor.
			 */
			Video(const Video& other) = default;

			/**
			 * Move constructor.
			 */
			Video(Video&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Video() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Video& operator=(const Video& other) = default;

			/**
			 * Move assignment.
			 */
			Video& operator=(Video&& other) = default;

			/**
			 * @return Color properties.
			 */
			const Property::Color& Color() const noexcept;

			/**
			 * @return Resolution.
			 */
			const Property::Resolution& Resolution() const noexcept;

			/**
			 * @return Optional HDR10 data.
			 */
			const std::optional<Property::HDR10>& HDR10() const noexcept;

			/**
			 * @return Cloned context.
			 */
			PointerType Clone() const override;

			/**
			 * @return Moved context as new pointer.
			 */
			PointerType Move() override;

		private:
			Property::Color m_color;						///< Color properties
			Property::Resolution m_resolution;				///< Frame size
			std::optional<Property::HDR10> m_hdr10;			///< Optional HDR10
	};
}
