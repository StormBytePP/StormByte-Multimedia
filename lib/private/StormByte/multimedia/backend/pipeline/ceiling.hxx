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

#include <StormByte/multimedia/visibility.h>

#include <cstddef>

#ifdef WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#elifdef MACOS
#	include <mach/mach.h>
#	include <sys/sysctl.h>
#else
#	include <fstream>
#	include <string>
#	include <unistd.h>
#endif

namespace StormByte::Multimedia::Backend::Pipeline {
	inline std::size_t AvailableRam() noexcept {
#ifdef WINDOWS
		MEMORYSTATUSEX st{};
		st.dwLength = sizeof(st);
		if (GlobalMemoryStatusEx(&st) != 0)
			return static_cast<std::size_t>(st.ullAvailPhys);
		return 0;
#elifdef MACOS
		mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
		vm_statistics64_data_t vm{};
		if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
				reinterpret_cast<host_info64_t>(&vm), &count) == KERN_SUCCESS) {
			const std::size_t page = static_cast<std::size_t>(vm_page_size);
			return (static_cast<std::size_t>(vm.free_count)
				+ static_cast<std::size_t>(vm.inactive_count)) * page;
		}
		std::size_t total = 0;
		std::size_t len = sizeof(total);
		const int mib[2] = { CTL_HW, HW_MEMSIZE };
		if (sysctl(mib, 2, &total, &len, nullptr, 0) == 0)
			return total / 2ull;
		return 0;
#else
		std::ifstream mem("/proc/meminfo");
		if (mem.is_open()) {
			std::string key, unit;
			std::size_t kb = 0;
			while (mem >> key >> kb >> unit) {
				if (key == "MemAvailable:")
					return kb * 1024ull;
			}
		}
		const long pages = ::sysconf(_SC_PHYS_PAGES);
		const long page = ::sysconf(_SC_PAGE_SIZE);
		if (pages > 0 && page > 0)
			return static_cast<std::size_t>(pages) * static_cast<std::size_t>(page) / 2ull;
		return 0;
#endif
	}

	/**
	 * @brief Hopper cap from available RAM.
	 * @param frames True = raw frames. False = packets.
	 * @param bias Divider after the raw n. Encoder 2, filter 1.
	 * @return Items. Never 0 (0 means unlimited at the call site).
	 *
	 * budget = available/8, frame = 4K P010, 6 queues.
	 * Frames clamped 2..32. Packets n*4 clamped 8..128.
	 */
	inline std::size_t SaneInputCeiling(bool frames, unsigned bias = 1) noexcept {
		const std::size_t ram = AvailableRam();
		if (ram == 0)
			return frames ? 8u : 32u;
		if (bias == 0)
			bias = 1;

		const std::size_t budget = ram / 8ull;
		const std::size_t bytes = 3840ull * 2160ull * 2ull;
		std::size_t n = budget / (bytes * 6ull);
		if (n < 2)
			n = 2;
		n /= bias;
		if (n < 2)
			n = 2;
		if (frames) {
			if (n > 32)
				n = 32;
			return n;
		}
		n *= 4ull;
		if (n < 8)
			n = 8;
		if (n > 128)
			n = 128;
		return n;
	}
}
