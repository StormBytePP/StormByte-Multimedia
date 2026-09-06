/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-Multimedia source in this
 * file. Third-party components — including FFmpeg and embedded trained data —
 * remain under their own licenses and are not covered by the commercial grant.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-Multimedia. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#pragma once

#include <StormByte/buffer/fifo.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>
#include <string>
#include <vector>

/**
 * @namespace StormByte::Multimedia
 * @brief Public multimedia types: codecs, containers, streams and files.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Attachment
	 * @brief Container attachment (cover, fonts). Not a Stream.
	 *
	 * avformat exposes Matroska attached files as fake video tracks
	 * (`AV_DISPOSITION_ATTACHED_PIC`). This type is the real contract.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Attachment {
		public:
			/**
			 * @brief Builds an attachment.
			 * @param fileName Source file name, if known.
			 * @param mimeType MIME type, if known.
			 * @param payload File bytes.
			 */
			Attachment(std::optional<std::string> fileName, std::optional<std::string> mimeType,
				StormByte::Buffer::FIFO payload) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Attachment(const Attachment&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Attachment(Attachment&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Attachment() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Attachment& operator=(const Attachment&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Attachment& operator=(Attachment&&) noexcept = default;

			/**
			 * @brief Original file name.
			 * @return Name, or empty.
			 */
			const std::optional<std::string>& FileName() const noexcept;

			/**
			 * @brief MIME type.
			 * @return Type, or empty.
			 */
			const std::optional<std::string>& MimeType() const noexcept;

			/**
			 * @brief Attachment bytes.
			 * @return FIFO.
			 */
			const StormByte::Buffer::FIFO& Payload() const noexcept;

		private:
			std::optional<std::string> m_fileName;	///< File name
			std::optional<std::string> m_mimeType;	///< MIME type
			StormByte::Buffer::FIFO m_payload;	///< Bytes
	};

	using Attachments = std::vector<Attachment>;
}
