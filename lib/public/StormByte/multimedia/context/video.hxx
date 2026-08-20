/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

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
