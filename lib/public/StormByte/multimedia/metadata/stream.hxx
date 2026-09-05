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

#include <StormByte/bitmask.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <optional>
#include <string>

namespace StormByte::Multimedia {
	/**
	 * @class File
	 * @brief Public media file snapshot.
	 */
	class File;

	/**
	 * @class Stream
	 * @brief Public media stream snapshot.
	 */
	class Stream;
}

namespace StormByte::Multimedia::Detail {
	/**
	 * @class Probe
	 * @brief Private metadata probe.
	 */
	class Probe;
}

/**
 * @namespace StormByte::Multimedia::Metadata
 * @brief Snapshot metadata for files and streams.
 */
namespace StormByte::Multimedia::Metadata {
	/**
	 * @class Stream
	 * @brief Per-stream tags and header fields captured at Open.
	 */
	class Stream;

	/**
	 * @enum DispositionFlag
	 * @brief Per-stream disposition bits (FFmpeg AV_DISPOSITION_* subset).
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC DispositionFlag: std::uint16_t {
		None			= 0,		///< No flags
		Default			= 1 << 0,	///< Default playback stream
		Dub			= 1 << 1,	///< Dubbed audio
		Original		= 1 << 2,	///< Original language
		Comment			= 1 << 3,	///< Commentary
		Lyrics			= 1 << 4,	///< Lyrics
		Karaoke			= 1 << 5,	///< Karaoke
		Forced			= 1 << 6,	///< Forced (e.g. foreign subs)
		HearingImpaired		= 1 << 7,	///< Hearing impaired
		VisualImpaired		= 1 << 8,	///< Visual impaired
		AttachedPicture		= 1 << 9	///< Cover / attached pic
	};

	/**
	 * @class Disposition
	 * @brief Bitmask of DispositionFlag.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Disposition: public StormByte::Bitmask<Disposition, DispositionFlag> {
		public:
			/**
			 * @brief Copy constructor.
			 * @param disposition Source mask.
			 */
			constexpr Disposition(const Disposition& disposition) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param disposition Source mask.
			 */
			constexpr Disposition(Disposition&& disposition) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~Disposition() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param disposition Source mask.
			 * @return *this.
			 */
			constexpr Disposition& operator=(const Disposition& disposition) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param disposition Source mask.
			 * @return *this.
			 */
			constexpr Disposition& operator=(Disposition&& disposition) noexcept = default;

		private:
			friend class Stream;
			friend class StormByte::Multimedia::File;
			friend class StormByte::Multimedia::Detail::Probe;

			/**
			 * @brief Empty mask.
			 */
			constexpr Disposition() noexcept
			: StormByte::Bitmask<Disposition, DispositionFlag>() {}

			/**
			 * @brief Mask from a single flag.
			 * @param flag Initial flag.
			 */
			constexpr Disposition(DispositionFlag flag) noexcept
			: StormByte::Bitmask<Disposition, DispositionFlag>(flag) {}
	};

	/**
	 * @class Stream
	 * @brief Per-stream tags and header fields captured at Open.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @brief Copy constructor.
			 */
			Stream(const Stream&) = default;

			/**
			 * @brief Move constructor.
			 */
			Stream(Stream&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Stream() = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			Stream& operator=(const Stream&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Stream& operator=(Stream&&) noexcept = default;

			/**
			 * @brief Stream title.
			 * @return Title, or empty.
			 */
			const std::optional<std::string>& Title() const noexcept;

			/**
			 * @brief Language code (usually ISO 639).
			 * @return Language, or empty.
			 */
			const std::optional<std::string>& Language() const noexcept;

			/**
			 * @brief Stream bitrate in bits per second.
			 * @return Bitrate, or empty.
			 */
			std::optional<std::uint64_t> BitRate() const noexcept;

			/**
			 * @brief Disposition flags.
			 * @return Mask (may be empty).
			 */
			class Disposition Disposition() const noexcept;

		private:
			friend class StormByte::Multimedia::File;
			friend class StormByte::Multimedia::Stream;
			friend class StormByte::Multimedia::Detail::Probe;

			std::optional<std::string> m_title;		///< title
			std::optional<std::string> m_language;		///< language
			std::optional<std::uint64_t> m_bitRate;		///< bit_rate
			class Disposition m_disposition;		///< disposition

			/**
			 * @brief Empty metadata.
			 */
			Stream() noexcept = default;

			/**
			 * @brief Sets the stream title.
			 * @param title Title tag.
			 */
			void Title(std::string title) noexcept;

			/**
			 * @brief Sets the language.
			 * @param language Language tag.
			 */
			void Language(std::string language) noexcept;

			/**
			 * @brief Sets the stream bitrate.
			 * @param bitRate Bits per second.
			 */
			void BitRate(std::uint64_t bitRate) noexcept;

			/**
			 * @brief Sets the disposition mask.
			 * @param disposition Flags.
			 */
			void Disposition(class Disposition disposition) noexcept;
	};
}
