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

#include <StormByte/iterable.hxx>
#include <StormByte/multimedia/visibility.h>

#include <map>
#include <string>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Metadata
	 * @brief Key/value metadata map (file or stream tags).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Metadata final: public StormByte::Iterable<std::map<std::string, std::string>> {
		public:
			/**
			 * Default constructor.
			 */
			Metadata() noexcept = default;

			/**
			 * Copy constructor.
			 */
			Metadata(const Metadata& other) = default;

			/**
			 * Move constructor.
			 */
			Metadata(Metadata&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Metadata() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Metadata& operator=(const Metadata& other) = default;

			/**
			 * Move assignment.
			 */
			Metadata& operator=(Metadata&& other) = default;
	};
}
