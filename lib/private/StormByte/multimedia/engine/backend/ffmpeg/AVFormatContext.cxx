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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>

#include <string>

using namespace StormByte::Multimedia::Engine::Backend;

FFmpeg::AVFormatContext::AVFormatContext(::AVFormatContext* ctx) noexcept:
AVPointer(ctx) {}

FFmpeg::AVFormatContext::~AVFormatContext() noexcept {
	Free();
}

FFmpeg::ExpectedAVFormatContext FFmpeg::AVFormatContext::Open(const std::filesystem::path& path) {
	::AVFormatContext* raw_ctx = nullptr;
	int ret;

	av_log_set_level(AV_LOG_ERROR);

	if ((ret = avformat_open_input(&raw_ctx, path.string().c_str(), nullptr, nullptr)) < 0)
		return Unexpected<DecoderError>("Could not open file {}: {}", path.string(), ErrorToString(ret));

	::AVFormatContext* fmt_ctx = raw_ctx;

	if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) < 0) {
		avformat_close_input(&fmt_ctx);
		return Unexpected<DecoderError>("Could not find stream information: {}", ErrorToString(ret));
	}

	return AVFormatContext(fmt_ctx);
}

const char* FFmpeg::AVFormatContext::FormatName() const noexcept {
	if (!m_ptr || !m_ptr->iformat)
		return nullptr;
	return m_ptr->iformat->name;
}

FFmpeg::OperationResult FFmpeg::AVFormatContext::ReadPacket(AVPacket& packet) noexcept {
	packet.Unref();
	int ret = av_read_frame(m_ptr, packet.Get());
	switch(ret) {
		case AVERROR(EAGAIN):
			return OperationResult::TryAgain;
		case AVERROR_EOF:
			return OperationResult::EndOfFile;
		case 0:
			return OperationResult::Success;
		default:
			return OperationResult::Error;
	}
}

FFmpeg::Streams FFmpeg::AVFormatContext::Streams() const noexcept {
	FFmpeg::Streams out;

	if (!m_ptr || m_ptr->nb_streams == 0)
		return out;

	for (unsigned i = 0; i < m_ptr->nb_streams; ++i) {
		out.emplace(AVStream(m_ptr->streams[i]));
	}

	return out;
}

std::optional<FFmpeg::AVBSF> FFmpeg::AVFormatContext::Mp4ToAnnexB(int codec_id, int stream_index, const AVCodecParameters& params) const noexcept {
	if (!m_ptr || !m_ptr->iformat || !m_ptr->iformat->name)
		return std::nullopt;

	const std::string fmt_name = m_ptr->iformat->name;

	bool is_mp4_like =
		fmt_name.find("mp4") != std::string::npos ||
		fmt_name.find("isom") != std::string::npos ||
		fmt_name.find("mov") != std::string::npos;

	if (!is_mp4_like)
		return std::nullopt;

	std::string bsf_name;
	switch (codec_id) {
		case AV_CODEC_ID_HEVC: bsf_name = "hevc_mp4toannexb"; break;
		case AV_CODEC_ID_H264: bsf_name = "h264_mp4toannexb"; break;
		case AV_CODEC_ID_AV1:  bsf_name = "av1_mp4toannexb"; break;
		default:               return std::nullopt;
	}

	auto expected_bsf = FFmpeg::AVBSF::Create(
		bsf_name,
		params,
		m_ptr->streams[stream_index]->time_base
	);

	if (expected_bsf)
		return std::move(expected_bsf.value());
	else
		return std::nullopt;
}

void FFmpeg::AVFormatContext::Free() noexcept {
	if (m_ptr) {
		avformat_close_input(&m_ptr);
		m_ptr = nullptr;
	}
}

template class StormByte::Multimedia::Engine::Backend::FFmpeg::AVPointer<::AVFormatContext>;
