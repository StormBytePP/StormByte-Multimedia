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

#include <StormByte/multimedia/engine/exception.hxx>
#include <StormByte/multimedia/visibility.h>

#include <string>
#include <string_view>

/**
 * @namespace OCR
 * @brief OCR-related engine utilities.
 */
namespace StormByte::Multimedia::Engine::OCR {
	/**
	 * @class TessDataNotFoundException
	 * @brief Thrown when requested tessdata language data is missing or unsupported.
	 *
	 * Internal (private ABI) exception. Constructed with the language code so
	 * Unexpected<TessDataNotFoundException>(...) and direct construction both work.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE TessDataNotFoundException: public StormByte::Multimedia::Engine::Exception {
		public:
			/**
			 * @brief Construct from a language code.
			 * @param lang Requested language identifier (e.g. "eng", "chi_tra_vert").
			 */
			explicit TessDataNotFoundException(std::string_view lang):
				StormByte::Multimedia::Engine::Exception(
					"OCR",
					"Unsupported or missing language: {}",
					std::string(lang)
				) {}

			/**
			 * Inherit remaining constructors from Engine::Exception
			 * (needed if Unexpected formats a message and constructs from std::string).
			 */
			using StormByte::Multimedia::Engine::Exception::Exception;

			/**
			 * Destructor.
			 */
			~TessDataNotFoundException() noexcept override = default;
	};
}