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

#include <StormByte/multimedia/pipeline/step.hxx>

using namespace StormByte::Multimedia::Pipeline;

Step::Step() noexcept
: m_failed(false) {}

Step::~Step() noexcept {
	if (m_worker.joinable())
		m_worker.request_stop();
}

void Step::Fail(std::string reason) noexcept {
	m_error = std::move(reason);
	m_failed.store(true, std::memory_order_release);
	m_in.Eof();
	m_out.Eof();
	m_wake.notify_all();
}

bool Step::Failed() const noexcept {
	return m_failed.load(std::memory_order_acquire);
}

const std::optional<std::string>& Step::Error() const noexcept {
	return m_error;
}

std::condition_variable& Step::Wake() noexcept {
	return m_wake;
}

void Step::Wait() noexcept {
	std::unique_lock lock(m_wait);
	m_wake.wait(lock, [this] {
		return Failed() || m_in.Ready();
	});
}

void Step::Open() noexcept {}

void Step::Work(std::shared_ptr<Item>) noexcept {}

void Step::Finish() noexcept {}

void Step::Pump() noexcept {
	for (;;) {
		if (Failed())
			break;
		std::shared_ptr<Item> item = m_in.Pop();
		if (!item) {
			if (!m_in.EoF()) {
				Wait();
				continue;
			}
			Finish();
			break;
		}
		Work(std::move(item));
	}
	m_out.Eof();
}

void Step::Launch() noexcept {
	if (m_worker.joinable())
		return;
	m_in.Wake(m_wake);
	m_worker = std::jthread([this]() {
		Open();
		Pump();
		m_out.Eof();
	});
}
