#pragma once

#include <multimedia/visibility.h>

#include <optional>
#include <string>
#include <utility>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @class Color
	 * @brief The class for color properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Color {
		public:
			static const Color 							DefaultForHDR10;	///< The default color for HDR10.

			using Point = std::pair<int, int>;			///< Representation for a point

			/**
			 * @brief Default constructor.
			 */
			Color()										= default;

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
			const std::optional<std::string>&			GetPrimaries() const noexcept;

			/**
			 * @brief Sets the primaries.
			 * @param primaries The primaries.
			 */
			void										SetPrimaries(const std::string&);

			/**
			 * @brief Sets the primaries.
			 * @param primaries The primaries.
			 */
			void										SetPrimaries(std::string&&) noexcept;

			/**
			 * @brief Gets the matrix.
			 * @return The matrix.
			 */
			const std::optional<std::string>& 			GetMatrix() const noexcept;

			/**
			 * @brief Sets the matrix.
			 * @param matrix The matrix.
			 */
			void										SetMatrix(const std::string&);

			/**
			 * @brief Sets the matrix.
			 * @param matrix The matrix.
			 */
			void										SetMatrix(std::string&&) noexcept;

			/**
			 * @brief Gets the transfer.
			 * @return The transfer.
			 */
			const std::optional<std::string>& 			GetTransfer() const noexcept;

			/**
			 * @brief Sets the transfer.
			 * @param transfer The transfer.
			 */
			void										SetTransfer(const std::string&);

			/**
			 * @brief Sets the transfer.
			 * @param transfer The transfer.
			 */
			void										SetTransfer(std::string&&) noexcept;

			/**
			 * @brief Gets the pixel format.
			 * @return The pixel format.
			 */
			const std::optional<std::string>&			GetPixelFormat() const noexcept;

			/**
			 * @brief Sets the pixel format.
			 * @param pix_fmt The pixel format.
			 */
			void										SetPixelFormat(const std::string&);

			/**
			 * @brief Sets the pixel format.
			 * @param pix_fmt The pixel format.
			 */
			void										SetPixelFormat(std::string&&) noexcept;

			/**
			 * @brief Checks if the color is HDR10.
			 * @return True if the color is HDR10, false otherwise.
			 */
			bool										IsHDR10Possible() const noexcept;

		private:
			std::optional<std::string> m_prim, m_matrix, m_transfer, m_pix_fmt;
	};
}