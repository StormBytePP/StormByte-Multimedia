/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia is dual-licensed under the following terms:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You can redistribute it and/or modify it under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this software may be used under the terms of a
 *    commercial license agreement with the sole copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *    Contact the copyright holder for more information.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/clonable.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Context
 * @brief Media stream context types (audio, video, …).
 */
namespace StormByte::Multimedia::Context {
	/**
	 * @class Generic
	 * @brief Base class for stream contexts (Clonable).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Generic: public StormByte::Clonable<Generic> {
		public:
			/**
			 * Default constructor.
			 */
			Generic() noexcept = default;

			/**
			 * Copy constructor.
			 */
			Generic(const Generic& other) = default;

			/**
			 * Move constructor.
			 */
			Generic(Generic&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			virtual ~Generic() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Generic& operator=(const Generic& other) = default;

			/**
			 * Move assignment.
			 */
			Generic& operator=(Generic&& other) = default;
	};
}
