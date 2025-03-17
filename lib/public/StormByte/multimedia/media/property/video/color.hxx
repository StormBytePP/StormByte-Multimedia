#pragma once

#include <StormByte/multimedia/visibility.h>

#include <optional>
#include <string>
#include <utility>

/**
 * @namespace Video
 * @brief The namespace for multimedia video media types.
 */
namespace StormByte::Multimedia::Media::Property::Video {
	/**
	 * @class Color
	 * @brief The class for color properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Color {
		public:
			static const Color 							DefaultForHDR10;	///< The default color for HDR10.

			using Point = std::pair<int, int>;			///< Representation for a point

			/**
			 * @brief Constructor.
			 * @param primaries The primaries.
			 * @param matrix The matrix.
			 * @param transfer The transfer.
			 * @param pix_fmt The pixel format.
			 */
			Color(const std::string& primaries, const std::string& matrix, const std::string& transfer, const std::string& pix_fmt);

			/**
			 * @brief Constructor.
			 * @param primaries The primaries.
			 * @param matrix The matrix.
			 * @param transfer The transfer.
			 * @param pix_fmt The pixel format.
			 */
			Color(std::string&& primaries, std::string&& matrix, std::string&& transfer, std::string&& pix_fmt) noexcept;

			/**
			 * @brief Copy constructor
			 * @param color The Color to copy.
			 */
			Color(const Color& color)					= default;

			/**
			 * @brief Move constructor.
			 * @param color The Color to move.
			 */
			Color(Color&& color) noexcept				= default;

			/**
			 * Copy assignment operator.
			 * @param color The Color to copy.
			 * @return The copied Color.
			 */
			Color& operator=(const Color& color)		= default;

			/**
			 * Move assignment operator.
			 * @param color The Color to move.
			 * @return The moved Color.
			 */
			Color& operator=(Color&& color) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Color() noexcept					= default;

			/**
			 * @brief Gets the primaries.
			 * @return The primaries.
			 */
			inline const std::optional<std::string>&	GetPrimaries() const noexcept {
				return m_prim;
			}

			/**
			 * @brief Sets the primaries.
			 * @param primaries The primaries.
			 */
			inline void									SetPrimaries(const std::string& primaries) {
				m_prim = primaries;
			} 

			/**
			 * @brief Sets the primaries.
			 * @param primaries The primaries.
			 */
			inline void									SetPrimaries(std::string&& primaries) noexcept {
				m_prim = std::move(primaries);
			}

			/**
			 * @brief Gets the matrix.
			 * @return The matrix.
			 */
			inline const std::optional<std::string>& 	GetMatrix() const noexcept {
				return m_matrix;
			}

			/**
			 * @brief Sets the matrix.
			 * @param matrix The matrix.
			 */
			inline void									SetMatrix(const std::string& matrix) {
				m_matrix = matrix;
			}

			/**
			 * @brief Sets the matrix.
			 * @param matrix The matrix.
			 */
			inline void									SetMatrix(std::string&& matrix) noexcept {
				m_matrix = std::move(matrix);
			}

			/**
			 * @brief Gets the transfer.
			 * @return The transfer.
			 */
			inline const std::optional<std::string>& 	GetTransfer() const noexcept {
				return m_transfer;
			}

			/**
			 * @brief Sets the transfer.
			 * @param transfer The transfer.
			 */
			inline void									SetTransfer(const std::string& transfer) {
				m_transfer = transfer;
			}

			/**
			 * @brief Sets the transfer.
			 * @param transfer The transfer.
			 */
			inline void									SetTransfer(std::string&& transfer) noexcept {
				m_transfer = std::move(transfer);
			}

			/**
			 * @brief Gets the pixel format.
			 * @return The pixel format.
			 */
			inline const std::optional<std::string>&	GetPixelFormat() const noexcept {
				return m_pix_fmt;
			}

			/**
			 * @brief Sets the pixel format.
			 * @param pix_fmt The pixel format.
			 */
			inline void									SetPixelFormat(const std::string& pix_fmt) {
				m_pix_fmt = pix_fmt;
			}

			/**
			 * @brief Sets the pixel format.
			 * @param pix_fmt The pixel format.
			 */
			inline void									SetPixelFormat(std::string&& pix_fmt) noexcept {
				m_pix_fmt = std::move(pix_fmt);
			}

			/**
			 * @brief Checks if the color is HDR10.
			 * @return True if the color is HDR10, false otherwise.
			 */
			bool										IsHDR10Possible() const noexcept;

		private:
			std::optional<std::string> m_prim, m_matrix, m_transfer, m_pix_fmt;
	};
}