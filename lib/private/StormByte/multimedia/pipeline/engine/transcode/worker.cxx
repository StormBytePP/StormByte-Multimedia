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

#include <StormByte/multimedia/pipeline/engine/transcode/engine.hxx>

#include <StormByte/logger/typedefs.hxx>
#include <StormByte/multimedia/pipeline/copy.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/stream.hxx>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <map>
#include <utility>

using StormByte::Logger::Level;
using StormByte::Multimedia::Type;
using StormByte::Multimedia::Pipeline::Packet;
using StormByte::Multimedia::Pipeline::Status;
using StormByte::Multimedia::Pipeline::Transcode;

namespace StormByte::Multimedia::Pipeline::Engine::Transcode {
	namespace {
		std::string FormatElapsed(std::chrono::steady_clock::time_point start) noexcept {
			const auto total = std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::steady_clock::now() - start).count();
			const auto hours = total / 3600;
			const auto minutes = (total % 3600) / 60;
			const auto seconds = total % 60;
			char buffer[16];
			if (hours > 0)
				std::snprintf(buffer, sizeof(buffer), "%lld:%02lld:%02lld",
					static_cast<long long>(hours),
					static_cast<long long>(minutes),
					static_cast<long long>(seconds));
			else if (minutes > 0)
				std::snprintf(buffer, sizeof(buffer), "%lld:%02lld",
					static_cast<long long>(minutes),
					static_cast<long long>(seconds));
			else
				std::snprintf(buffer, sizeof(buffer), "%lld",
					static_cast<long long>(seconds));
			return buffer;
		}
	}

	void Engine::Run(StormByte::Multimedia::Pipeline::Transcode& owner) noexcept {
		auto& log = *owner.m_logger;
		log << Level::LowLevel << "Transcode::Worker enter" << std::endl;
		owner.OnConfigure();
		if (status.load(std::memory_order_acquire) == Status::Error) {
			owner.OnError(owner.Error().value_or("configure failed"));
			return;
		}
		if (cancel.load(std::memory_order_acquire)) {
			status.store(Status::Aborted, std::memory_order_release);
			owner.OnAborted();
			return;
		}
		if (!container || path.empty()) {
			owner.Fail("destination is not set");
			owner.OnError(owner.Error().value_or("destination is not set"));
			return;
		}

		if (auto snapshot = owner.Configuration())
			owner.OnPlan(*snapshot);

		std::vector<Slot> plan = mapped;
		for (const auto& slot : plan) {
			if (!slot.copy && slot.codec == nullptr) {
				owner.Fail("stream " + std::to_string(slot.in) + " has no Codec() or Copy()");
				owner.OnError(owner.Error().value_or("incomplete map"));
				return;
			}
		}

		std::set<int> used;
		for (const auto& slot : plan)
			used.insert(slot.in);
		for (int in : ignore)
			used.insert(in);
		for (const auto& stream : owner.m_file->Streams()) {
			if (used.contains(stream.Index()))
				continue;
			Slot implied;
			implied.in = stream.Index();
			implied.outKey = stream.Index();
			implied.kind = stream.Type();
			implied.copy = true;
			plan.push_back(std::move(implied));
		}

		std::sort(plan.begin(), plan.end(), [](const Slot& a, const Slot& b) {
			if (a.outKey != b.outKey)
				return a.outKey < b.outKey;
			return a.in < b.in;
		});

		const auto start = owner.OnStart();
		if (start != Status::Running) {
			if (start == Status::Error) {
				if (!owner.Failed())
					owner.Fail("OnStart rejected the job");
				owner.OnError(owner.Error().value_or("OnStart rejected the job"));
			}
			else if (start == Status::Aborted) {
				status.store(Status::Aborted, std::memory_order_release);
				owner.OnAborted();
			}
			else {
				status.store(Status::Stopped, std::memory_order_release);
			}
			return;
		}

		class Demux demux;
		*owner.m_file >> demux;
		if (demux.Failed()) {
			owner.Fail(demux.Error().value_or("demux open failed"));
			owner.OnError(owner.Error().value_or("demux"));
			return;
		}

		class Mux mux(*container);
		mux >> path;
		*owner.m_file >> mux;
		if (mux.Failed()) {
			owner.Fail(mux.Error().value_or("mux open failed"));
			owner.OnError(owner.Error().value_or("mux"));
			return;
		}

		struct EncodeLane {
			int in = -1;
			Type kind = Type::Video;
			std::unique_ptr<class Decoder> decoder;
			std::unique_ptr<class Encoder> encoder;
			PacketQueue* packets = nullptr;
			FrameQueue* frames = nullptr;
		};

		std::vector<std::unique_ptr<class Copy>> copies;
		std::vector<EncodeLane> lanes;
		std::map<int, PacketQueue*> encodeByIn;
		std::set<int> copyIn;
		muxQueue = std::make_unique<PacketQueue>(owner.m_muxPacketCeiling);
		copyQueue = std::make_unique<PacketQueue>(owner.m_copyPacketCeiling);
		encodeQueues.clear();
		frameQueues.clear();

		int muxIndex = 0;
		for (auto& slot : plan) {
			if (slot.copy) {
				auto copy = std::make_unique<class Copy>(muxIndex, slot.in);
				demux >> *copy;
				*copy >> mux;
				copyIn.insert(slot.in);
				copies.push_back(std::move(copy));
			}
			else {
				auto decoder = std::make_unique<class Decoder>(slot.in);
				auto encoder = std::make_unique<class Encoder>(muxIndex, *slot.codec);
				if (slot.implementation)
					encoder->Implementation(*slot.implementation);
				if (slot.crf)
					encoder->CRF(*slot.crf);
				if (slot.bitRate)
					encoder->BitRate(*slot.bitRate);
				if (slot.maxBitRate)
					encoder->MaxBitRate(*slot.maxBitRate);
				if (slot.preset)
					encoder->Preset(*slot.preset);
				if (slot.tune)
					encoder->Tune(*slot.tune);
				if (!slot.fineTune.empty())
					encoder->FineTune(slot.fineTune);
				if (slot.language)
					encoder->Language(*slot.language);
				if (slot.title)
					encoder->Title(*slot.title);
				demux >> *decoder;
				*encoder >> mux;
				const auto ceiling = slot.kind == Type::Video
					? owner.m_videoPacketCeiling : owner.m_packetCeiling;
				encodeQueues.push_back(std::make_unique<PacketQueue>(ceiling));
				EncodeLane lane;
				lane.in = slot.in;
				lane.kind = slot.kind;
				lane.decoder = std::move(decoder);
				lane.encoder = std::move(encoder);
				lane.packets = encodeQueues.back().get();
				if (slot.kind == Type::Video) {
					frameQueues.push_back(std::make_unique<FrameQueue>(owner.m_videoFrameCeiling));
					lane.frames = frameQueues.back().get();
				}
				encodeByIn[slot.in] = lane.packets;
				lanes.push_back(std::move(lane));
			}
			++muxIndex;
		}

		log << Level::Notice << "transcode running tracks=" << plan.size()
			<< " encode-lanes=" << lanes.size()
			<< " video-frames=" << owner.m_videoFrameCeiling
			<< " video-packets=" << owner.m_videoPacketCeiling
			<< " mux-packets=" << owner.m_muxPacketCeiling
			<< " copy-packets=" << owner.m_copyPacketCeiling << std::endl;
		const auto started = std::chrono::steady_clock::now();
		const auto duration = owner.m_file->Duration();

		auto drainEncoder = [this, &owner](class Encoder& encoder, std::deque<class Packet>& held) -> bool {
			while (!held.empty()) {
				if (!muxQueue->TryPush(std::move(held.front())))
					return true;
				held.pop_front();
				NotifyIntake();
			}
			for (;;) {
				if (muxQueue->Full())
					return true;
				class Packet encoded;
				encoder >> encoded;
				if (encoder.Failed()) {
					owner.Fail(encoder.Error().value_or("encoder failed"));
					return false;
				}
				if (encoded.StreamIndex() < 0)
					break;
				if (!muxQueue->TryPush(std::move(encoded))) {
					held.push_back(std::move(encoded));
					return true;
				}
				NotifyIntake();
			}
			return true;
		};

		auto drainDecoderToFrames = [this, &owner, duration](class Decoder& decoder, FrameQueue& frames) -> bool {
			for (;;) {
				class Frame frame;
				decoder >> frame;
				if (decoder.Failed()) {
					owner.Fail(decoder.Error().value_or("decoder failed"));
					return false;
				}
				if (frame.StreamIndex() < 0)
					break;
				if (frame.Pts() && duration) {
					const auto pts = frame.Pts()->Nanoseconds().count();
					const auto total = duration->Nanoseconds().count();
					if (total > 0)
						owner.SetProgress(static_cast<unsigned>((pts * 100) / total));
				}
				frames.Push(std::move(frame), cancel);
			}
			return true;
		};

		auto encodeOneFrame = [this, &owner, &drainEncoder](class Encoder& encoder, class Frame& frame,
			int in, std::deque<class Packet>& held) -> bool {
			frame >> encoder;
			if (encoder.Failed()) {
				owner.Fail(encoder.Error().value_or("encoder failed"));
				return false;
			}
			owner.MarkSettled(in, encoder);
			return drainEncoder(encoder, held);
		};

		auto flushHeld = [this](std::deque<class Packet>& held) {
			while (!held.empty() && !cancel.load(std::memory_order_acquire)) {
				muxQueue->Push(std::move(held.front()), cancel);
				held.pop_front();
				NotifyIntake();
			}
		};

		std::thread muxThread([this, &owner, &mux]() {
			while (!cancel.load(std::memory_order_acquire)) {
				WaitIfPaused();
				if (cancel.load(std::memory_order_acquire))
					break;
				std::optional<class Packet> packet = muxQueue->TryPop();
				if (!packet)
					packet = copyQueue->TryPop();
				if (!packet) {
					if (muxQueue->Closed() && copyQueue->Closed())
						break;
					WaitForIntake();
					continue;
				}
				if (packet->StreamIndex() < 0)
					continue;
				*packet >> mux;
				if (mux.Failed()) {
					owner.Fail(mux.Error().value_or("mux write failed"));
					break;
				}
			}
		});

		std::vector<std::thread> workers;
		for (auto& lane : lanes) {
			if (lane.frames) {
				workers.emplace_back([this, &owner, &lane, &drainDecoderToFrames]() {
					while (!cancel.load(std::memory_order_acquire)) {
						WaitIfPaused();
						if (cancel.load(std::memory_order_acquire))
							break;
						auto packet = lane.packets->Pop(cancel);
						if (!packet)
							break;
						if (packet->StreamIndex() < 0)
							continue;
						*packet >> *lane.decoder;
						if (lane.decoder->Failed()) {
							owner.Fail(lane.decoder->Error().value_or("decoder failed"));
							break;
						}
						if (!drainDecoderToFrames(*lane.decoder, *lane.frames))
							break;
					}
					if (!owner.Failed() && !cancel.load(std::memory_order_acquire)) {
						lane.decoder->Flush();
						if (lane.decoder->Failed())
							owner.Fail(lane.decoder->Error().value_or("decoder flush failed"));
						else
							(void)drainDecoderToFrames(*lane.decoder, *lane.frames);
					}
					lane.frames->Close();
				});
				workers.emplace_back([this, &owner, &lane, &encodeOneFrame, &drainEncoder, &flushHeld]() {
					std::deque<class Packet> held;
					while (!cancel.load(std::memory_order_acquire)) {
						WaitIfPaused();
						if (cancel.load(std::memory_order_acquire))
							break;
						auto frame = lane.frames->Pop(cancel);
						if (!frame)
							break;
						if (frame->StreamIndex() < 0)
							continue;
						if (!encodeOneFrame(*lane.encoder, *frame, lane.in, held))
							break;
					}
					if (!owner.Failed() && !cancel.load(std::memory_order_acquire)) {
						lane.encoder->Flush();
						if (lane.encoder->Failed())
							owner.Fail(lane.encoder->Error().value_or("encoder flush failed"));
						else
							(void)drainEncoder(*lane.encoder, held);
						flushHeld(held);
					}
				});
			}
			else {
				workers.emplace_back([this, &owner, &lane, &drainEncoder, &flushHeld]() {
					std::deque<class Packet> held;
					auto pump = [&]() -> bool {
						for (;;) {
							class Frame frame;
							*lane.decoder >> frame;
							if (lane.decoder->Failed()) {
								owner.Fail(lane.decoder->Error().value_or("decoder failed"));
								return false;
							}
							if (frame.StreamIndex() < 0)
								break;
							frame >> *lane.encoder;
							if (lane.encoder->Failed()) {
								owner.Fail(lane.encoder->Error().value_or("encoder failed"));
								return false;
							}
							owner.MarkSettled(lane.in, *lane.encoder);
							if (!drainEncoder(*lane.encoder, held))
								return false;
						}
						return true;
					};
					while (!cancel.load(std::memory_order_acquire)) {
						WaitIfPaused();
						if (cancel.load(std::memory_order_acquire))
							break;
						auto packet = lane.packets->Pop(cancel);
						if (!packet)
							break;
						if (packet->StreamIndex() < 0)
							continue;
						*packet >> *lane.decoder;
						if (lane.decoder->Failed()) {
							owner.Fail(lane.decoder->Error().value_or("decoder failed"));
							break;
						}
						if (!pump())
							break;
					}
					if (!owner.Failed() && !cancel.load(std::memory_order_acquire)) {
						lane.decoder->Flush();
						if (!lane.decoder->Failed())
							(void)pump();
						lane.encoder->Flush();
						if (!lane.encoder->Failed())
							(void)drainEncoder(*lane.encoder, held);
						flushHeld(held);
					}
				});
			}
		}

		std::thread demuxThread([this, &owner, &demux, &encodeByIn, &copyIn]() {
			while (!cancel.load(std::memory_order_acquire) && demux) {
				WaitIfPaused();
				if (cancel.load(std::memory_order_acquire))
					break;
				class Packet packet;
				demux >> packet;
				if (demux.Failed()) {
					owner.Fail(demux.Error().value_or("demux read failed"));
					break;
				}
				if (demux.Eof() || packet.StreamIndex() < 0)
					break;
				const int in = packet.StreamIndex();
				if (ignore.contains(in))
					continue;
				if (copyIn.contains(in)) {
					copyQueue->Push(std::move(packet), cancel);
					NotifyIntake();
					continue;
				}
				auto found = encodeByIn.find(in);
				if (found == encodeByIn.end())
					continue;
				found->second->Push(std::move(packet), cancel);
			}
			for (auto& queue : encodeQueues)
				queue->Close();
			copyQueue->Close();
			muxQueue->Close();
			NotifyIntake();
		});

		demuxThread.join();
		for (auto& thread : workers)
			thread.join();
		muxQueue->Close();
		copyQueue->Close();
		NotifyIntake();
		muxThread.join();

		if (cancel.load(std::memory_order_acquire)
			&& status.load(std::memory_order_acquire) != Status::Error) {
			status.store(Status::Aborted, std::memory_order_release);
			log << Level::Warning << "transcode aborted in "
				<< FormatElapsed(started) << std::endl;
			owner.OnAborted();
			return;
		}

		if (!owner.Failed()) {
			mux.Flush();
			if (mux.Failed())
				owner.Fail(mux.Error().value_or("mux flush failed"));
		}

		if (owner.Failed()) {
			owner.OnError(owner.Error().value_or("transcode failed"));
			return;
		}

		owner.SetProgress(100);
		status.store(Status::Done, std::memory_order_release);
		log << Level::Notice << "transcode done in " << FormatElapsed(started) << std::endl;
		owner.OnDone();
	}
}
