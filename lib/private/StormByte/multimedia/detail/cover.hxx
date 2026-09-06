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

#include <StormByte/multimedia/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVStream.hxx>

#include <chrono>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
}

/**
 * @namespace StormByte::Multimedia::Detail
 * @brief Private probe helpers.
 */
namespace StormByte::Multimedia::Detail {
	/**
	 * @brief Still-image codecs used as covers.
	 * @param codecId `AVCodecID`.
	 * @return true for MJPEG/PNG/… .
	 */
	inline bool IsStillImageCodec(int codecId) noexcept {
		switch (static_cast<AVCodecID>(codecId)) {
			case AV_CODEC_ID_MJPEG:
			case AV_CODEC_ID_MJPEGB:
			case AV_CODEC_ID_PNG:
			case AV_CODEC_ID_APNG:
			case AV_CODEC_ID_BMP:
			case AV_CODEC_ID_GIF:
			case AV_CODEC_ID_WEBP:
			case AV_CODEC_ID_TIFF:
			case AV_CODEC_ID_JPEG2000:
			case AV_CODEC_ID_PAM:
			case AV_CODEC_ID_PPM:
			case AV_CODEC_ID_JPEGLS:
				return true;
			default:
				return false;
		}
	}

	/**
	 * @brief FFmpeg attached-pic flag.
	 * @param stream Stream view.
	 * @return true if `AV_DISPOSITION_ATTACHED_PIC`.
	 */
	inline bool IsAttachedPicture(const Backend::FFmpeg::AVStream& stream) noexcept {
		return (stream.Disposition() & AV_DISPOSITION_ATTACHED_PIC) != 0;
	}

	/**
	 * @brief Matroska attachment stream (`ATTACHMENT` / `NONE`).
	 * @param stream Stream view.
	 * @return true if it is not a media track.
	 */
	inline bool IsContainerAttachment(const Backend::FFmpeg::AVStream& stream) noexcept {
		if (stream.Type() == AVMEDIA_TYPE_ATTACHMENT)
			return true;
		return stream.CodecParameters().CodecId() == AV_CODEC_ID_NONE;
	}

	/**
	 * @brief true if the container has a real video track.
	 * @param ctx Opened format context.
	 * @return true when a non-cover video exists.
	 */
	inline bool HasPrimaryVideo(const Backend::FFmpeg::AVFormatContext& ctx) noexcept {
		for (const auto& stream : ctx.Streams()) {
			if (stream.Type() != AVMEDIA_TYPE_VIDEO)
				continue;
			if (IsAttachedPicture(stream) || IsContainerAttachment(stream))
				continue;
			if (IsStillImageCodec(stream.CodecParameters().CodecId()))
				continue;
			return true;
		}
		return false;
	}

	/**
	 * @brief Cover-like video (flag or remuxed still next to a primary video).
	 * @param stream Stream view.
	 * @param hasPrimaryVideo Result of HasPrimaryVideo.
	 * @return true if it must not be a Stream.
	 */
	inline bool IsCoverStream(const Backend::FFmpeg::AVStream& stream, bool hasPrimaryVideo) noexcept {
		if (IsAttachedPicture(stream))
			return true;
		if (!hasPrimaryVideo)
			return false;
		if (stream.Type() != AVMEDIA_TYPE_VIDEO)
			return false;
		if (!IsStillImageCodec(stream.CodecParameters().CodecId()))
			return false;
		const auto duration = stream.Duration();
		if (!duration.has_value() || *duration <= std::chrono::milliseconds{50})
			return true;
		return false;
	}

	/**
	 * @brief Packet/stream index that File maps to Attachments().
	 * @param ctx Opened format context.
	 * @param index avformat stream index.
	 * @return true if Demux must drop it and Decoder must fail.
	 */
	inline bool IsAttachmentIndex(const Backend::FFmpeg::AVFormatContext& ctx, int index) noexcept {
		const bool hasPrimaryVideo = HasPrimaryVideo(ctx);
		for (const auto& stream : ctx.Streams()) {
			if (stream.Index() != index)
				continue;
			return IsContainerAttachment(stream) || IsCoverStream(stream, hasPrimaryVideo);
		}
		return false;
	}
}
