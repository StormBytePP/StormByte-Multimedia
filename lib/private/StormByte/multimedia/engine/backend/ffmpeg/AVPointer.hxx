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
#include <StormByte/type_traits.hxx>

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVPointer
	 * @brief Move-only RAII base for FFmpeg C pointers.
	 * @tparam AVType Underlying FFmpeg struct type.
	 *
	 * Derived classes must implement Free().
	 */
	template<typename AVType>
	class STORMBYTE_MULTIMEDIA_PRIVATE AVPointer {
		public:
			/**
			 * @brief Default constructor (deleted).
			 */
			constexpr AVPointer() noexcept = delete;

			/**
			 * @brief Copy constructor (deleted).
			 */
			constexpr AVPointer(const AVPointer&) noexcept = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source wrapper.
			 */
			constexpr AVPointer(AVPointer&& other) noexcept
			: m_ptr(other.m_ptr) {
				other.m_ptr = nullptr;
			}

			/**
			 * @brief Destructor. Does not free; derived Free() owns that.
			 */
			virtual ~AVPointer() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			constexpr AVPointer& operator=(const AVPointer&) noexcept = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source wrapper.
			 * @return *this.
			 */
			constexpr AVPointer& operator=(AVPointer&& other) noexcept {
				if (this != &other) {
					Free();
					m_ptr = other.m_ptr;
					other.m_ptr = nullptr;
				}
				return *this;
			}

			/**
			 * @brief Const view of the raw pointer.
			 * @return Pointer or nullptr.
			 */
			constexpr const std::decay_t<AVType>* Get() const noexcept {
				return m_ptr;
			}

		protected:
			std::decay_t<AVType>* m_ptr = nullptr;	///< Owned FFmpeg pointer

			/**
			 * @brief Adopts @p ptr.
			 * @param ptr Raw pointer.
			 */
			explicit constexpr AVPointer(std::decay_t<AVType>* ptr) noexcept
			: m_ptr(ptr) {}

			/**
			 * @brief Mutable view of the raw pointer.
			 * @return Pointer or nullptr.
			 */
			constexpr std::decay_t<AVType>* Get() noexcept {
				return m_ptr;
			}

			/**
			 * @brief Releases the underlying resource.
			 */
			virtual void Free() noexcept = 0;
	};
}
