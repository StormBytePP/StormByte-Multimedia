#pragma once

#include <array>
#include <cstddef>
#include <span>

/**
 * @namespace StormByte::Multimedia::Media::Tables::Codec
 * @brief Private static codec tables (StormByte name + FFmpeg ids).
 */
namespace StormByte::Multimedia::Media::Tables::Codec {
	/**
	 * @struct CodecDef
	 * @brief One catalog row.
	 */
	struct CodecDef {
		const char* name;				///< StormByte name
		const char* description;			///< Description
		std::array<const char*, 4> ffmpegIds;	///< FFmpeg ids; unused slots nullptr

		/**
		 * @brief Number of non-null FFmpeg ids.
		 * @return Count in `[0, 4]`.
		 */
		constexpr std::size_t FfmpegIdCount() const noexcept {
			std::size_t n = 0;
			for (const char* id : ffmpegIds) {
				if (!id)
					break;
				++n;
			}
			return n;
		}

		/**
		 * @brief FFmpeg id at @p index.
		 * @param index Zero-based index.
		 * @return Id, or nullptr if out of range.
		 */
		constexpr const char* FfmpegId(std::size_t index) const noexcept {
			if (index >= FfmpegIdCount())
				return nullptr;
			return ffmpegIds[index];
		}
	};

	/**
	 * @brief Video codec rows.
	 * @return Span over the video table.
	 */
	std::span<const CodecDef> Video() noexcept;

	/**
	 * @brief Audio codec rows.
	 * @return Span over the audio table.
	 */
	std::span<const CodecDef> Audio() noexcept;

	/**
	 * @brief Subtitle codec rows.
	 * @return Span over the subtitle table.
	 */
	std::span<const CodecDef> Subtitle() noexcept;

	/**
	 * @brief Attachment codec rows.
	 * @return Span over the attachment table.
	 */
	std::span<const CodecDef> Attachment() noexcept;
}
