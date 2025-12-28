#include <StormByte/multimedia/codec.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia;

Codec::Codec(int codec_id, const std::string& name, const std::string& description) noexcept:
m_codec_id(codec_id), m_name(name), m_description(description) {}

std::string Codec::Name() const noexcept {
	return m_name;
}

std::string Codec::Description() const noexcept {
	return m_description;
}

StormByte::Multimedia::ExpectedCodec Codec::Find(const std::string& name) noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;

	while ((c = av_codec_iterate(&opaque))) {
		if (strcmp(c->name, name.c_str()) == 0) {
			return Codec(
				c->id,
				c->name,
				c->long_name ? c->long_name : ""
			);
		}
	}

	return Unexpected<CodecNotFound>(name);
}

StormByte::Multimedia::ExpectedCodec Codec::Find(int id) noexcept {
	AVCodecID codec_id = static_cast<AVCodecID>(id);

	void* opaque = nullptr;
	const AVCodec* c = nullptr;
	const AVCodec* fallback = nullptr;

	// Prefer decoder
	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == codec_id) {
			if (!fallback)
				fallback = c;

			if (av_codec_is_decoder(c)) {
				return Codec(
					codec_id,
					c->name,
					c->long_name ? c->long_name : ""
				);
			}
		}
	}

	// Fallback: first implementation
	if (fallback) {
		return Codec(
			codec_id,
			fallback->name,
			fallback->long_name ? fallback->long_name : ""
		);
	}

	return Unexpected<CodecNotFound>(std::to_string(id));
}

bool Codec::HasDecoder() const noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;

	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == m_codec_id && av_codec_is_decoder(c))
			return true;
	}

	return false;
}

bool Codec::HasEncoder() const noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;

	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == m_codec_id && av_codec_is_encoder(c))
			return true;
	}

	return false;
}