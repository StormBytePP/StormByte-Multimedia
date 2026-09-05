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

//==============================================================================
// FILE: lib/private/StormByte/multimedia/engine/backend/ffmpeg/demuxer.cxx
//==============================================================================

#include <StormByte/multimedia/context/audio.hxx>
#include <StormByte/multimedia/context/video.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/demuxer.hxx>
#include <StormByte/multimedia/engine/codec.hxx>
#include <StormByte/multimedia/engine/stream.hxx>

#include <cstring>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/dict.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/mastering_display_metadata.h>
}

using namespace StormByte::Multimedia::Engine::Backend;

ExpectedDemuxerTuple FFmpeg::Demuxer::Open(const std::filesystem::path& path) const noexcept {
	if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
		return Unexpected<ReadError>("File does not exist or is not a regular file");
	}

	av_log_set_level(AV_LOG_ERROR);

	auto expected_fmt = FFmpeg::AVFormatContext::Open(path);
	if (!expected_fmt)
		return Unexpected<ContentError>(expected_fmt.error()->what());

	FFmpeg::AVFormatContext fmt = std::move(expected_fmt.value());

	StormByte::Multimedia::Metadata metadata = fmt.Metadata();
	FFmpeg::Streams av_streams = fmt.Streams();
	Engine::Streams streams;

	for (const auto& av_stream : av_streams) {
		const FFmpeg::AVCodecParameters par = av_stream.CodecParameters();
		if (!par.Get())
			continue;

		auto expected_codec = Codec::Find(par.Get()->codec_id);
		if (!expected_codec)
			continue;

		const Codec& codec = *expected_codec;
		std::shared_ptr<Stream> stream;

		switch (av_stream.Type()) {
			case AVMEDIA_TYPE_AUDIO: {
				stream = std::make_shared<Stream>(codec, StormByte::Multimedia::Type::Audio);

				Context::Audio context(
					par.Get()->sample_rate,
					par.Get()->ch_layout.nb_channels,
					par.Get()->bit_rate,
					std::nullopt
				);
				stream->Context(std::move(context));
				break;
			}
			case AVMEDIA_TYPE_VIDEO: {
				stream = std::make_shared<Stream>(codec, StormByte::Multimedia::Type::Video);

				const char* pix_fmt_name = av_get_pix_fmt_name(
					static_cast<AVPixelFormat>(par.Get()->format)
				);
				const char* range_name = av_color_range_name(par.Get()->color_range);
				const char* space_name = av_color_space_name(par.Get()->color_space);

				std::string primaries;
				if (par.Get()->color_primaries != AVCOL_PRI_UNSPECIFIED) {
					const char* primaries_name = av_color_primaries_name(par.Get()->color_primaries);
					if (primaries_name)
						primaries = primaries_name;
				}
				std::string transfer;
				if (par.Get()->color_trc != AVCOL_TRC_UNSPECIFIED) {
					const char* transfer_name = av_color_transfer_name(par.Get()->color_trc);
					if (transfer_name)
						transfer = transfer_name;
				}

				Context::Property::Resolution resolution(
					static_cast<unsigned short>(par.Get()->width),
					static_cast<unsigned short>(par.Get()->height)
				);
				Context::Property::Color color(
					pix_fmt_name ? pix_fmt_name : "unknown",
					range_name ? range_name : "unknown",
					space_name ? space_name : "unknown",
					primaries,
					transfer
				);
				double fps = av_stream.FrameRate();
				const bool has_hdr10 = color.IsHDR10Possible();

				std::optional<Context::Property::HDR10> hdr10_opt;

				if (has_hdr10) {
					std::optional<Context::Property::Point> red_point, green_point, blue_point, white_point, luminance, light_level;

					const AVCodec* dec_codec = avcodec_find_decoder(par.Get()->codec_id);
					if (dec_codec) {
						auto expected_dec = FFmpeg::AVDecoder::Open(
							const_cast<AVCodec*>(dec_codec), par, fmt, av_stream.Index()
						);
						if (expected_dec) {
							FFmpeg::AVDecoder dec = std::move(expected_dec.value());
							FFmpeg::AVPacket pkt;
							FFmpeg::AVFrame frame;

							bool got_metadata = false;
							bool got_mastering_display = false;
							bool got_content_light_level = false;
							bool hdr10_plus_detected = false;
							const int packet_limit = static_cast<int>(fps > 0.0 ? fps * 5.0 : 150.0);
							int packets_read = 0;

							while (!got_metadata && packets_read < packet_limit) {
								auto res = fmt.ReadPacket(pkt);
								if (res != FFmpeg::OperationResult::Success)
									break;
								packets_read++;

								if (pkt.StreamIndex() == av_stream.Index()) {
									auto send_res = dec.SendPacket(pkt);
									if (send_res == FFmpeg::OperationResult::Success) {
										while (dec.ReceiveFrame(frame) == FFmpeg::OperationResult::Success) {
											if (const AVFrameSideData* sd = frame.SideData(AV_FRAME_DATA_MASTERING_DISPLAY_METADATA)) {
												const auto* md = reinterpret_cast<const AVMasteringDisplayMetadata*>(sd->data);

												red_point = Context::Property::Point::Normalized(
													md->display_primaries[0][0].num, md->display_primaries[0][0].den,
													md->display_primaries[0][1].num, md->display_primaries[0][1].den,
													50000
												);
												green_point = Context::Property::Point::Normalized(
													md->display_primaries[1][0].num, md->display_primaries[1][0].den,
													md->display_primaries[1][1].num, md->display_primaries[1][1].den,
													50000
												);
												blue_point = Context::Property::Point::Normalized(
													md->display_primaries[2][0].num, md->display_primaries[2][0].den,
													md->display_primaries[2][1].num, md->display_primaries[2][1].den,
													50000
												);
												white_point = Context::Property::Point::Normalized(
													md->white_point[0].num, md->white_point[0].den,
													md->white_point[1].num, md->white_point[1].den,
													50000
												);

												const unsigned int max_lum_num = md->has_luminance ? md->max_luminance.num : 0;
												const unsigned int max_lum_den = md->has_luminance ? md->max_luminance.den : 1;
												const unsigned int min_lum_num = md->has_luminance ? md->min_luminance.num : 0;
												const unsigned int min_lum_den = md->has_luminance ? md->min_luminance.den : 1;

												luminance = Context::Property::Point::Normalized(
													min_lum_num, min_lum_den, max_lum_num, max_lum_den, 10000
												);
												got_mastering_display = true;
											}

											if (const AVFrameSideData* sd = frame.SideData(AV_FRAME_DATA_CONTENT_LIGHT_LEVEL)) {
												const auto* cll = reinterpret_cast<const AVContentLightMetadata*>(sd->data);
												light_level = Context::Property::Point(cll->MaxCLL, cll->MaxFALL);
												got_content_light_level = true;
											}

											if (frame.SideData(AV_FRAME_DATA_DYNAMIC_HDR_PLUS))
												hdr10_plus_detected = true;

											got_metadata = got_mastering_display || got_content_light_level || hdr10_plus_detected;
											if (got_metadata)
												break;
										}
									}
								}
								pkt.Unref();
							}

							if (red_point && green_point && blue_point && white_point) {
								Context::Property::HDR10 hd(
									std::move(*red_point),
									std::move(*green_point),
									std::move(*blue_point),
									std::move(*white_point),
									luminance ? std::move(*luminance) : Context::Property::Point(0, 0),
									std::move(light_level)
								);
								hd.HDR10Plus(hdr10_plus_detected);
								hdr10_opt = std::move(hd);
							} else {
								Context::Property::HDR10 def = Context::Property::HDR10::DEFAULT;
								def.HDR10Plus(hdr10_plus_detected);
								hdr10_opt = std::move(def);
							}
						}
					}
				}

				Context::Video vctx(std::move(color), std::move(resolution), std::move(hdr10_opt));
				stream->Context(std::move(vctx));
				break;
			}
			case AVMEDIA_TYPE_SUBTITLE: {
				stream = std::make_shared<Stream>(codec, StormByte::Multimedia::Type::Subtitle);
				break;
			}
			default: {
				stream = std::make_shared<Stream>(codec, StormByte::Multimedia::Type::Unknown);
				break;
			}
		}

		if (!stream)
			continue;

		stream->Metadata(av_stream.Metadata());
		streams.add(std::move(stream));
	}

	return std::make_tuple(std::move(metadata), std::move(streams));
}