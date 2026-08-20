#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>
#include <utility>

/**
 * @namespace Property
 * @brief Video/audio property value types.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class Color
	 * @brief Pixel format and colorimetry strings.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Color final {
		public:
			/**
			 * Full color description.
			 * @param pix_fmt Pixel format name.
			 * @param range Color range name.
			 * @param space Color space name.
			 * @param primaries Color primaries name.
			 * @param transfer Transfer characteristics name.
			 */
			Color(const std::string& pix_fmt, const std::string& range, const std::string& space,
				const std::string& primaries, const std::string& transfer) noexcept;

			/**
			 * Minimal color description (primaries/transfer empty).
			 * @param pix_fmt Pixel format name.
			 * @param range Color range name.
			 * @param space Color space name.
			 */
			Color(std::string&& pix_fmt, std::string&& range, std::string&& space) noexcept;

			/**
			 * Copy constructor.
			 */
			Color(const Color& color) = default;

			/**
			 * Move constructor.
			 */
			Color(Color&& color) noexcept = default;

			/**
			 * Copy assignment.
			 */
			Color& operator=(const Color& color) = default;

			/**
			 * Move assignment.
			 */
			Color& operator=(Color&& color) noexcept = default;

			/**
			 * Destructor.
			 */
			virtual ~Color() noexcept = default;

			/**
			 * @return Pixel format name.
			 */
			const std::string& PixelFormat() const noexcept;

			/**
			 * @return Color range name.
			 */
			const std::string& Range() const noexcept;

			/**
			 * @return Color space name.
			 */
			const std::string& Space() const noexcept;

			/**
			 * @return Transfer characteristics name.
			 */
			const std::string& Transfer() const noexcept;

			/**
			 * @return Color primaries name.
			 */
			const std::string& Primaries() const noexcept;

			/**
			 * @return true if format/range/space look like HDR10-capable.
			 */
			bool IsHDR10Possible() const noexcept;

			/**
			 * @return true if format/range/space look like HLG-capable.
			 */
			bool IsHLGPossible() const noexcept;

		protected:
			std::string m_pix_fmt;		///< Pixel format
			std::string m_range;		///< Range
			std::string m_space;		///< Space
			std::string m_primaries;	///< Primaries
			std::string m_transfer;		///< Transfer
	};
}
