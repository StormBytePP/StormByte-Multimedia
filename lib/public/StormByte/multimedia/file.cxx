#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/context/audio.hxx>
#include <StormByte/multimedia/context/video.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>

#include <cstring>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/dict.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/mastering_display_metadata.h>
}

using namespace StormByte::Multimedia;

File::File(const std::filesystem::path& path) noexcept:
m_path(path) {}

const std::filesystem::path& File::Path() const noexcept {
	return m_path;
}

const Metadata& File::Metadata() const noexcept {
	return m_metadata;
}

ExpectedFile File::Open(const std::filesystem::path& path) noexcept {
	// Set less verbose logging
	av_log_set_level(AV_LOG_ERROR);

	// Open using RAII wrapper
	auto expected_fmt = FFmpeg::AVFormatContext::Open(path);
	if (!expected_fmt)
		return Unexpected<FileError>(expected_fmt.error()->what());

	// Move the FFmpeg context out of the expected
	FFmpeg::AVFormatContext fmt = std::move(expected_fmt.value());

	// Create file object
	File file(path);

	// Retrieve metadata from RAII object
	file.m_metadata = fmt.Metadata();

	// Get streams set from RAII object
	auto ff_streams = fmt.Streams();

	// Reserve space
	file.m_streams.reserve(ff_streams.size());

	// Iterate over each stream in the file (FFmpeg::AVStream wrappers)
	for (const auto& av_stream : ff_streams) {
		auto expected_codec = Codec::Find(av_stream.CodecParameters()->codec_id); // may be used later

		// Construct a Stream object and set its context according to type
		Stream stream{*expected_codec, StormByte::Multimedia::Type::Unknown};

		// Read stream data and properties
		switch(av_stream.Type()) {
			case AVMEDIA_TYPE_AUDIO: {
				// Create Audio stream object
				stream = Stream{*expected_codec, StormByte::Multimedia::Type::Audio};

				// Set basic context properties (no profile lookup available here)
				const AVCodecParameters* par = av_stream.CodecParameters();
				Context::Audio context(
					par->sample_rate,
					par->ch_layout.nb_channels,
					par->bit_rate,
					std::nullopt
				);

				stream.Context(std::move(context));
				break;
			}
			case AVMEDIA_TYPE_VIDEO: {
				// Create Video stream object
				stream = Stream{*expected_codec, StormByte::Multimedia::Type::Video};

				// Set basic context properties
				const AVCodecParameters* par = av_stream.CodecParameters();
				const char* pix_fmt_name = av_get_pix_fmt_name(
					static_cast<AVPixelFormat>(par->format)
				);
				const char* range_name = av_color_range_name(
					par->color_range
				);
				const char* space_name = av_color_space_name(
					par->color_space
				);
				// Get primaries and transfer characteristics (used for Color)
				std::string primaries = "";
				if (par->color_primaries != AVCOL_PRI_UNSPECIFIED) {
					const char* primaries_name = av_color_primaries_name(par->color_primaries);
					if (primaries_name)
						primaries = primaries_name;
				}
				std::string transfer = "";
				if (par->color_trc != AVCOL_TRC_UNSPECIFIED) {
					const char* transfer_name = av_color_transfer_name(par->color_trc);
					if (transfer_name)
						transfer = transfer_name;
				}
				Context::Property::Resolution resolution(
					static_cast<unsigned short>(par->width),
					static_cast<unsigned short>(par->height)
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

				// Prepare optional HDR10 property
				std::optional<Context::Property::HDR10> hdr10_opt;

				// Check for HDR10 metadata
				if (has_hdr10) {
					// Define the data holders
					std::optional<Context::Property::Point> red_point, green_point, blue_point, white_point, luminance, light_level;

					// Initialize decoder to read frames via RAII
					const AVCodecParameters* par = av_stream.CodecParameters();
					const AVCodec* codec = avcodec_find_decoder(par->codec_id);
					if (!codec)
						break;

					auto expected_dec = FFmpeg::AVDecoder::Open(const_cast<AVCodec*>(codec), const_cast<AVCodecParameters*>(par), fmt, av_stream.Index());
					if (!expected_dec)
						break;
					FFmpeg::AVDecoder dec = std::move(expected_dec.value());

					FFmpeg::AVPacket pkt;
					FFmpeg::AVFrame frame;

					// Limiters to not read the whole file
					bool got_metadata = false;
					bool got_mastering_display = false;
					bool got_content_light_level = false;
					bool hdr10_plus_detected = false;
					int packet_limit = static_cast<int>(fps * 5); // Read 5 seconds
					int packets_read = 0;

					// Read packets until obtaining a frame with side data
					while (!got_metadata && packets_read < packet_limit) {
						auto res = fmt.ReadPacket(pkt);
						if (res != FFmpeg::OperationResult::Success)
							break;
						packets_read++;
						if (pkt.StreamIndex() == av_stream.Index()) {
							auto send_res = dec.SendPacket(pkt);
							if (send_res == FFmpeg::OperationResult::Success) {
								while (dec.ReceiveFrame(frame) == FFmpeg::OperationResult::Success) {
									// Mastering Display Metadata
									if (const AVFrameSideData* sd = frame.SideData(AV_FRAME_DATA_MASTERING_DISPLAY_METADATA)) {
										const AVMasteringDisplayMetadata* md = (const AVMasteringDisplayMetadata*)sd->data;

										// Primaries
										red_point = Context::Property::Point::Normalized(
											md->display_primaries[0][0].num,
											md->display_primaries[0][0].den,
											md->display_primaries[0][1].num,
											md->display_primaries[0][1].den,
											50000
										);

										green_point = Context::Property::Point::Normalized(
											md->display_primaries[1][0].num,
											md->display_primaries[1][0].den,
											md->display_primaries[1][1].num,
											md->display_primaries[1][1].den,
											50000
										);

										blue_point = Context::Property::Point::Normalized(
											md->display_primaries[2][0].num,
											md->display_primaries[2][0].den,
											md->display_primaries[2][1].num,
											md->display_primaries[2][1].den,
											50000
										);

										white_point = Context::Property::Point::Normalized(
											md->white_point[0].num,
											md->white_point[0].den,
											md->white_point[1].num,
											md->white_point[1].den,
											50000
										);

										// Luminance
										unsigned int max_lum_num = md->has_luminance
											? md->max_luminance.num
											: 0;
										unsigned int max_lum_den = md->has_luminance
											? md->max_luminance.den
											: 1;

										unsigned int min_lum_num = md->has_luminance
											? md->min_luminance.num
											: 0;
										unsigned int min_lum_den = md->has_luminance
											? md->min_luminance.den
											: 1;

										luminance = Context::Property::Point::Normalized(
											min_lum_num,
											min_lum_den,
											max_lum_num,
											max_lum_den,
											10000
										);

										got_mastering_display = true;
									}

									// Content Light Level
									if (const AVFrameSideData* sd = frame.SideData(AV_FRAME_DATA_CONTENT_LIGHT_LEVEL)) {
										const AVContentLightMetadata* cll =
											(const AVContentLightMetadata*)sd->data;

										light_level = Context::Property::Point(cll->MaxCLL, cll->MaxFALL);
										got_content_light_level = true;
									}

									// HDR10+ detection
									if (frame.SideData(AV_FRAME_DATA_DYNAMIC_HDR_PLUS)) {
										hdr10_plus_detected = true;
									}

									// Update got_metadata flag
									got_metadata = got_mastering_display || got_content_light_level || hdr10_plus_detected;

									if (got_metadata)
										break;
								}
							}
						}
						pkt.Unref();
					}
					
					// Set HDR10 metadata if available
					if (red_point.has_value() && green_point.has_value() &&
						blue_point.has_value() && white_point.has_value()) {
						Context::Property::HDR10 hd(
							std::move(*red_point),
							std::move(*green_point),
							std::move(*blue_point),
							std::move(*white_point),
							std::move(*luminance),
							std::move(light_level)
						);
						hd.HDR10Plus(hdr10_plus_detected);
						hdr10_opt = std::move(hd);
					}
					else {
						// No HDR10 metadata found, use default
						Context::Property::HDR10 def = Context::Property::HDR10::DEFAULT;
						def.HDR10Plus(hdr10_plus_detected);
						hdr10_opt = std::move(def);
					}
				}

				// Build video context and assign
				Context::Video vctx(std::move(color), std::move(resolution), std::move(hdr10_opt));
				stream.Context(std::move(vctx));
				break;
			}
			case AVMEDIA_TYPE_SUBTITLE: {
				// Create Subtitle stream object
				stream = Stream{*expected_codec, StormByte::Multimedia::Type::Subtitle};
				// No additional context for subtitles currently
				break;
			}
			default: {
				// For streams that we do not modify like subtitles, data, attachments, etc.
				stream = Stream{*expected_codec, StormByte::Multimedia::Type::Unknown};
				break;
			}
		}

		// Read stream metadata via RAII wrapper
		stream.Metadata(av_stream.Metadata());
		file.m_streams.push_back(std::move(stream));
	}

	// Return the File object
	return file;
}

Streams::const_iterator File::StreamsBegin() const noexcept {
	return m_streams.begin();
}

Streams::const_iterator File::StreamsEnd() const noexcept {
	return m_streams.end();
}