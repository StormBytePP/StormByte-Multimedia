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

#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/filters/report.hxx>
#include <StormByte/multimedia/pipeline/route.hxx>

using namespace StormByte::Multimedia::Pipeline;

Route::Route(int track, bool copy) noexcept
: m_track(track), m_copy(copy) {}

Route::~Route() noexcept = default;

int Route::Track() const noexcept {
	return m_track;
}

bool Route::Copy() const noexcept {
	return m_copy;
}

void Route::Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!filter)
		return;

    if (!dynamic_cast<Filter::Process*>(filter.get())
        && !dynamic_cast<Filter::Packet*>(filter.get())
        && !dynamic_cast<Filter::Analytics*>(filter.get())) {
        filter->Fail("inherit Process, Packet or Analytics; FFmpeg is not a leaf");
        return;
    }

	const auto accepts = filter->Accepts();
	const bool frame = accepts.Has(Kind::Frame);
	const bool packet = accepts.Has(Kind::Packet);
	if (m_copy && frame && !packet)
		return;

	Filter::FFmpeg& node = *filter;
	const bool analytics = dynamic_cast<Filter::Analytics*>(filter.get()) != nullptr;
	if (packet)
		Hook(m_packets, node, analytics);
	if (frame && !m_copy)
		Hook(m_frames, node, analytics);

	m_filters.push_back(std::move(filter));
	m_filters.back()->Launch();
}

void Route::Close(Step& origin, Step& destination) noexcept {
	Filter::FFmpeg* const packetFirst = m_packets.First();
	Filter::FFmpeg* const packetLast = m_packets.Last();
	Filter::FFmpeg* const frameFirst = m_frames.First();
	Filter::FFmpeg* const frameLast = m_frames.Last();

	if (packetLast != nullptr && frameFirst != nullptr)
		packetLast->m_out.Bind(m_track, frameFirst->m_in);

	Filter::FFmpeg* first = packetFirst != nullptr ? packetFirst : frameFirst;
	Filter::FFmpeg* last = frameLast != nullptr ? frameLast : packetLast;

	destination.m_in.Wake(destination.Wake());
	if (first == nullptr) {
		origin.m_out.Bind(m_track, destination.m_in);
	}
	else {
		first->m_in.Wake(first->Wake());
		origin.m_out.Bind(m_track, first->m_in);
		last->m_out.Bind(m_track, destination.m_in);
	}
}

std::vector<Filter::Report> Route::Reports() const noexcept {
	std::vector<Filter::Report> reports;
	reports.reserve(m_filters.size());
	for (const auto& filter : m_filters)
		reports.push_back(filter->Report());
	return reports;
}

void Route::Hook(Lane& lane, Filter::FFmpeg& filter, bool analytics) noexcept {
	filter.m_in.Wake(filter.Wake());
	if (analytics) {
		if (lane.LastAnalytics != nullptr)
			lane.LastAnalytics->m_out.Bind(m_track, filter.m_in);
		else if (lane.LastProcess != nullptr)
			lane.LastProcess->m_out.Bind(m_track, filter.m_in);
		if (lane.FirstAnalytics == nullptr)
			lane.FirstAnalytics = &filter;
		lane.LastAnalytics = &filter;
		return;
	}
	if (lane.LastProcess != nullptr)
		lane.LastProcess->m_out.Bind(m_track, filter.m_in);
	if (lane.FirstProcess == nullptr)
		lane.FirstProcess = &filter;
	lane.LastProcess = &filter;
	if (lane.FirstAnalytics != nullptr)
		filter.m_out.Bind(m_track, lane.FirstAnalytics->m_in);
}
