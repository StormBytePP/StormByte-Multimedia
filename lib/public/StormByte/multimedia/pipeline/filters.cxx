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

#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/filters.hxx>
#include <StormByte/multimedia/pipeline/plan.hxx>
#include <StormByte/multimedia/pipeline/remuxer.hxx>
#include <StormByte/multimedia/pipeline/route.hxx>
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/type.hxx>

#include <algorithm>
#include <map>
#include <string>

using namespace StormByte::Multimedia::Pipeline;

namespace {
	bool Terminal(State state) noexcept {
		return state == State::Stopped || state == State::Failed;
	}

	int HopperKey(Step& origin, Step& dest) noexcept {
		if (auto* decoder = dynamic_cast<Decoder*>(&origin))
			return decoder->Index();
		if (auto* remuxer = dynamic_cast<Remuxer*>(&dest))
			return remuxer->In();
		if (auto* encoder = dynamic_cast<Encoder*>(&dest))
			return encoder->Index();
		if (auto* encoder = dynamic_cast<Encoder*>(&origin))
			return encoder->Index();
		if (auto* remuxer = dynamic_cast<Remuxer*>(&origin))
			return remuxer->In();
		return -1;
	}

	enum StormByte::Multimedia::Type StreamMedia(const Step& origin, int track) noexcept {
		const auto& plan = origin.Plan();
		if (!plan)
			return StormByte::Multimedia::Type::Unknown;
		for (const auto& stream : plan->Source().Streams()) {
			if (stream.Index() == track)
				return stream.Type();
		}

		return StormByte::Multimedia::Type::Unknown;
	}

	bool Matches(const Filter::FFmpeg& filter, const Step& origin, Producer dest, int track) noexcept {
		if (dest != Producer::Encoder && dest != Producer::Remuxer)
			return false;
		const auto media = StreamMedia(origin, track);
		if (media != StormByte::Multimedia::Type::Unknown && filter.Media() != media)
			return false;
		return true;
	}

	std::string FlattenKey(const std::string& leaf, std::optional<int> track,
		std::map<std::string, int>& seen) noexcept {
		std::string base = leaf + "[";
		if (track)
			base += std::to_string(*track);
		else
			base += "general";
		base += "]";
		const int n = ++seen[base];
		if (n == 1)
			return base;
		return base + "#" + std::to_string(n);
	}
}

Filters::Filters() noexcept = default;

Filters::Handle::Handle(Filters& owner, std::size_t index) noexcept
: m_owner(&owner), m_index(index) {}

Filters::Handle& Filters::Handle::Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!filter || !m_owner || m_index >= m_owner->m_stretches.size())
		return *this;
	auto& stretch = m_owner->m_stretches[m_index];
	if (!stretch.Lane)
		return *this;

	if (dynamic_cast<Filter::ProcessTwoPasses*>(filter.get())) {
		if (dynamic_cast<Remuxer*>(stretch.Destination.get())) {
			stretch.Destination->Fail("ProcessTwoPasses cannot remux");
			return *this;
		}
		if (stretch.Track >= 0) {
			auto& tracks = m_owner->m_measureTracks;
			if (std::find(tracks.begin(), tracks.end(), stretch.Track) == tracks.end())
				tracks.push_back(stretch.Track);
		}
	}

	m_owner->m_reports.push_back({filter, stretch.Scope});
	stretch.Lane->Add(std::move(filter));
	return *this;
}

Filters::Handle Filters::Between(std::shared_ptr<Step> origin,
	std::shared_ptr<Step> destination) noexcept {
	Stretch stretch;
	stretch.Origin = std::move(origin);
	stretch.Destination = std::move(destination);
	if (stretch.Origin && stretch.Destination) {
		stretch.Track = HopperKey(*stretch.Origin, *stretch.Destination);
		stretch.Scope = stretch.Track >= 0 ? std::optional<int>(stretch.Track) : std::nullopt;
		if (stretch.Track < 0)
			stretch.Destination->Fail("cannot infer hopper key for Between");
		else
			stretch.Lane = std::make_unique<Route>(stretch.Track, stretch.Origin, stretch.Destination);
	}

	m_stretches.push_back(std::move(stretch));
	return Handle(*this, m_stretches.size() - 1);
}

Filters::~Filters() noexcept {
	for (auto& global : m_globals) {
		if (global.Filter)
			global.Filter->Halt();
	}
}

Filters& Filters::Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!filter)
		return *this;
	if (dynamic_cast<Filter::Analytics*>(filter.get()) == nullptr) {
		filter->Fail("global Add is Analytics; Process/Packet go on Between");
		return *this;
	}

	m_reports.push_back({filter, std::nullopt});
	filter->Launch();
	m_globals.push_back({std::move(filter), std::nullopt});
	return *this;
}

void Filters::Close() noexcept {
	for (auto& stretch : m_stretches) {
		if (!stretch.Lane || !stretch.Origin || !stretch.Destination)
			continue;
		for (auto& global : m_globals) {
			if (global.Filter && Matches(*global.Filter, *stretch.Origin,
				stretch.Destination->m_name, stretch.Track))
				stretch.Lane->Observe(*global.Filter);
		}

		stretch.Lane->Close();
	}

	if (m_measureTracks.empty())
		return;

	Demuxer* demuxer = nullptr;
	for (auto& stretch : m_stretches) {
		if (!stretch.Origin)
			continue;
		if (auto* decoder = dynamic_cast<Decoder*>(stretch.Origin.get())) {
			demuxer = decoder->m_origin;
			if (demuxer)
				break;
		}
		else if (auto* found = dynamic_cast<Demuxer*>(stretch.Origin.get())) {
			demuxer = found;
			break;
		}
	}
	if (!demuxer)
		return;

	m_measureFilterCount = 0;
	m_measureFiltersDrained = 0;
	for (auto& item : m_reports) {
		if (auto* two = dynamic_cast<Filter::ProcessTwoPasses*>(item.Filter.get())) {
			two->EnterMeasure();
			two->BindMeasure(this);
			++m_measureFilterCount;
		}
	}

	demuxer->Measure(m_measureTracks);
	demuxer->m_filters = this;
	m_measuring = true;
	m_measureDrained.clear();
}

bool Filters::Measuring() const noexcept {
	return m_measuring;
}

void Filters::CloseMeasureSource() noexcept {
	for (auto& stretch : m_stretches) {
		if (!stretch.Origin)
			continue;
		auto* decoder = dynamic_cast<Decoder*>(stretch.Origin.get());
		if (!decoder)
			continue;
		if (std::find(m_measureTracks.begin(), m_measureTracks.end(),
				decoder->Index()) == m_measureTracks.end())
			continue;
		decoder->MeasureSourceClosed();
	}

	for (auto& item : m_reports) {
		if (auto* two = dynamic_cast<Filter::ProcessTwoPasses*>(item.Filter.get()))
			two->MeasureSourceClosed();
	}
}

void Filters::OnMeasureDrained(int track) noexcept {
	if (std::find(m_measureDrained.begin(), m_measureDrained.end(), track)
			== m_measureDrained.end())
		m_measureDrained.push_back(track);
	MaybeFinishMeasure();
}

void Filters::OnMeasureFilterDrained() noexcept {
	++m_measureFiltersDrained;
	MaybeFinishMeasure();
}

void Filters::MaybeFinishMeasure() noexcept {
	if (m_measureDrained.size() < m_measureTracks.size())
		return;
	if (m_measureFiltersDrained < m_measureFilterCount)
		return;
	FinishMeasure();
}

void Filters::FinishMeasure() noexcept {
	if (!m_measuring)
		return;

	for (auto& item : m_reports) {
		if (auto* two = dynamic_cast<Filter::ProcessTwoPasses*>(item.Filter.get()))
			two->LeaveMeasure();
	}

	Demuxer* demuxer = nullptr;
	for (auto& stretch : m_stretches) {
		if (!stretch.Origin)
			continue;
		if (auto* decoder = dynamic_cast<Decoder*>(stretch.Origin.get())) {
			demuxer = decoder->m_origin;
			if (demuxer)
				break;
		}
		else if (auto* found = dynamic_cast<Demuxer*>(stretch.Origin.get())) {
			demuxer = found;
			break;
		}
	}
	if (!demuxer)
		return;
	if (!demuxer->Rewind())
		return;
	demuxer->Apply();
	m_measuring = false;
	m_measureDrained.clear();
	m_measureFiltersDrained = 0;
}

bool Filters::Idle() const noexcept {
	for (const auto& stretch : m_stretches) {
		if (stretch.Lane && !stretch.Lane->Idle())
			return false;
	}

	for (const auto& global : m_globals) {
		if (global.Filter && !Terminal(global.Filter->Status()))
			return false;
	}

	return true;
}

std::vector<std::pair<std::string, Filter::Report>> Filters::Reports() const noexcept {
	std::vector<std::pair<std::string, Filter::Report>> out;
	std::map<std::string, int> seen;
	out.reserve(m_reports.size());
	for (const auto& item : m_reports) {
		if (!item.Filter)
			continue;
		out.emplace_back(FlattenKey(item.Filter->Leaf(), item.Track, seen),
			item.Filter->Report());
	}

	return out;
}
