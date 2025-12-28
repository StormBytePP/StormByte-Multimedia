#pragma once

#include <StormByte/multimedia/context/generic.hxx>
#include <StormByte/multimedia/context/property/color.hxx>
#include <StormByte/multimedia/context/property/hdr10.hxx>
#include <StormByte/multimedia/context/property/resolution.hxx>

#include <optional>

/**
 * @namespace Context
 * @brief The namespace for all context classes.
 */
namespace StormByte::Multimedia::Context {
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Generic {
		public:
			/**
			 * @brief Constructor.
			 * @param color The color properties.
			 * @param resolution The resolution properties.
			 * @param hdr10 The HDR10 properties (optional).
			 */
			Video(Property::Color&& color, Property::Resolution&& resolution, std::optional<Property::HDR10>&& hdr10) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other The other context to copy from.
			 */
			Video(const Video& other)								= default;

			/**
			 * @brief Move constructor.
			 * @param other The other context to move from.
			 */
			Video(Video&& other) noexcept							= default;

			/**
			 * @brief Default destructor.
			 */
			~Video() noexcept 										= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other context to copy from.
			 * @return Reference to this context.
			 */
			Video& operator=(const Video& other)					= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other context to move from.
			 * @return Reference to this context.
			 */
			Video& operator=(Video&& other)							= default;

			/**
			 * @brief Gets the color properties.
			 * @return The color properties.
			 */
			const Property::Color&									Color() const noexcept;

			/**
			 * @brief Gets the resolution properties.
			 * @return The resolution properties.
			 */
			const Property::Resolution&								Resolution() const noexcept;

			/**
			 * @brief Gets the HDR10 properties.
			 * @return The HDR10 properties.
			 */
			const std::optional<Property::HDR10>&					HDR10() const noexcept;

			/**
			 * @brief Clones the object
			 * @return cloned object
			 */
			PointerType 											Clone() const override;

			/**
			 * @brief Moves the object
			 * @return moved object
			 */
			PointerType 											Move() override;

		private:
			Property::Color m_color;								///< The color properties.
			Property::Resolution m_resolution;						///< The resolution properties.
			std::optional<Property::HDR10> m_hdr10;					///< The HDR10 properties.
	};
}