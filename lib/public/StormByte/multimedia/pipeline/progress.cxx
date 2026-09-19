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

#include <StormByte/multimedia/pipeline/progress.hxx>

#include <format>

using StormByte::Multimedia::Pipeline::Progress;

double Progress::Axis(std::int64_t pos, std::int64_t dur) noexcept {
	if (dur <= 0 || pos <= 0)
		return 0.0;
	if (pos >= dur)
		return 100.0;
	return (static_cast<double>(pos) * 100.0) / static_cast<double>(dur);
}

std::optional<double> Progress::Measure() const noexcept {
	if (!m_hasMeasure)
		return std::nullopt;
	if (m_measureDone)
		return 100.0;
	return Axis(m_measureNs, m_durationNs);
}

std::optional<double> Progress::Analytics() const noexcept {
	if (!m_hasAnalytics)
		return std::nullopt;
	if (m_analyticsDone)
		return 100.0;
	return Axis(m_analyticsNs, m_durationNs);
}

bool Progress::HasMeasure() const noexcept {
	return m_hasMeasure;
}

bool Progress::HasAnalytics() const noexcept {
	return m_hasAnalytics;
}

bool Progress::MeasureComplete() const noexcept {
	return !m_hasMeasure || m_measureDone;
}

bool Progress::AnalyticsComplete() const noexcept {
	return !m_hasAnalytics || m_analyticsDone;
}

double Progress::All() const noexcept {
	if (m_muxDone && MeasureComplete() && AnalyticsComplete()) {
		m_all = 100.0;
		return m_all;
	}

	double sum = 0.0;
	unsigned n = 0;

	if (m_hasMeasure) {
		sum += m_measureDone ? 100.0 : Axis(m_measureNs, m_durationNs);
		++n;
	}

	sum += m_muxDone ? 100.0 : Axis(m_passNs, m_durationNs);
	++n;

	if (m_hasAnalytics) {
		sum += m_analyticsDone ? 100.0 : Axis(m_analyticsNs, m_durationNs);
		++n;
	}

	double raw = n == 0 ? 0.0 : sum / static_cast<double>(n);

	if (m_hasAnalytics && !m_analyticsDone) {
		const double lag = Axis(m_analyticsNs, m_durationNs);
		if (raw > lag)
			raw = lag;
	}

	if (raw < m_all)
		raw = m_all;
	if (m_hasAnalytics && !m_analyticsDone) {
		const double lag = Axis(m_analyticsNs, m_durationNs);
		if (raw > lag)
			raw = lag;
	}

	m_all = raw;
	return m_all;
}

Progress::operator std::string() const noexcept {
	std::string line;
	const bool measureLive = m_hasMeasure && !m_measureDone;
	if (const auto v = Measure(); v && measureLive)
		line += std::format("measure {:6.2f}%  ", *v);
	if (!measureLive) {
		if (const auto v = Analytics(); v && !m_analyticsDone)
			line += std::format("analytics {:6.2f}%  ", *v);
	}
	line += std::format("all {:6.2f}%", All());
	return line;
}

void Progress::HasMeasure(bool on) noexcept {
	m_hasMeasure = on;
}

void Progress::HasAnalytics(bool on) noexcept {
	m_hasAnalytics = on;
}

void Progress::SetDurationNs(std::int64_t ns) noexcept {
	if (ns > 0)
		m_durationNs = ns;
}

void Progress::SetMeasureNs(std::int64_t ns) noexcept {
	if (ns < 0)
		return;
	if (m_durationNs > 0 && ns > m_durationNs)
		ns = m_durationNs;
	if (ns < m_measureNs)
		return;
	m_measureNs = ns;
}

void Progress::SetPassNs(std::int64_t ns) noexcept {
	if (ns < 0)
		return;
	if (m_durationNs > 0 && ns > m_durationNs)
		ns = m_durationNs;
	if (ns < m_passNs)
		return;
	m_passNs = ns;
}

void Progress::SetAnalyticsNs(std::int64_t ns) noexcept {
	if (ns < 0)
		return;
	if (m_durationNs > 0 && ns > m_durationNs)
		ns = m_durationNs;
	if (ns < m_analyticsNs)
		return;
	m_analyticsNs = ns;
}

void Progress::MeasureDone() noexcept {
	m_measureDone = true;
	if (m_durationNs > 0)
		m_measureNs = m_durationNs;
}

void Progress::PassDone() noexcept {
	m_passDone = true;
	if (m_durationNs > 0)
		m_passNs = m_durationNs;
}

void Progress::MuxDone() noexcept {
	m_muxDone = true;
	m_analyticsAtMux = m_analyticsNs;
}

void Progress::AnalyticsDone() noexcept {
	m_analyticsDone = true;
	if (m_durationNs > 0)
		m_analyticsNs = m_durationNs;
}
