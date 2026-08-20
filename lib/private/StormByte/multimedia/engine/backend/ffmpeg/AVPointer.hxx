/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/type_traits.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
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
			 * Default constructor (deleted).
			 */
			constexpr AVPointer() noexcept = delete;

			/**
			 * Copy constructor (deleted).
			 */
			constexpr AVPointer(const AVPointer&) noexcept = delete;

			/**
			 * Move constructor.
			 * @param other Source pointer wrapper.
			 */
			constexpr AVPointer(AVPointer&& other) noexcept
			: m_ptr(other.m_ptr) {
				other.m_ptr = nullptr;
			}

			/**
			 * Destructor (does not free; derived Free() is responsible).
			 */
			virtual ~AVPointer() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			constexpr AVPointer& operator=(const AVPointer&) noexcept = delete;

			/**
			 * Move assignment.
			 * @param other Source pointer wrapper.
			 * @return *this
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
			 * @return Const underlying pointer.
			 */
			constexpr const std::decay_t<AVType>* Get() const noexcept {
				return m_ptr;
			}

		protected:
			std::decay_t<AVType>* m_ptr = nullptr;	///< Owned FFmpeg pointer

			/**
			 * @param ptr Raw pointer to adopt.
			 */
			explicit constexpr AVPointer(std::decay_t<AVType>* ptr) noexcept
			: m_ptr(ptr) {}

			/**
			 * @return Mutable underlying pointer.
			 */
			constexpr std::decay_t<AVType>* Get() noexcept {
				return m_ptr;
			}

			/**
			 * Releases the underlying resource.
			 */
			virtual void Free() noexcept = 0;
	};
}
