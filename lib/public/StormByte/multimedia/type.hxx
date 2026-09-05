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

#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @enum Type
	 * @brief Media stream / content type.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type {
		Audio,						///< Audio media type
		Video,						///< Video media type
		Subtitle,					///< Subtitle media type
		Attachment,					///< Attachment media type
		Copy,						///< Stream copy (passthrough)
		Unknown						///< Unknown media type
	};

	/**
	 * Converts a Type to a string.
	 * @param type Value to convert.
	 * @return Null-terminated string literal.
	 */
	constexpr const char* ToString(Type type) noexcept {
		switch (type) {
			case Type::Audio:		return "Audio";
			case Type::Video:		return "Video";
			case Type::Subtitle:	return "Subtitle";
			case Type::Attachment:	return "Attachment";
			case Type::Copy:		return "Copy";
			case Type::Unknown:		return "Unknown";
			default:				return "Invalid";
		}
	}
}
