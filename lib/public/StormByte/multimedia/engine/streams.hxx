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

#include <StormByte/iterable.hxx>
#include <StormByte/multimedia/engine/stream.hxx>

#include <memory>
#include <vector>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Streams
	 * @brief Ordered collection of Stream shared_ptrs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Streams final: public Iterable<std::vector<std::shared_ptr<Stream>>> {
		public:
			/**
			 * Default constructor.
			 */
			Streams() noexcept = default;

			/**
			 * Copy constructor.
			 */
			Streams(const Streams& other) = default;

			/**
			 * Move constructor.
			 */
			Streams(Streams&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Streams() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Streams& operator=(const Streams& other) = default;

			/**
			 * Move assignment.
			 */
			Streams& operator=(Streams&& other) noexcept = default;
	};
}
