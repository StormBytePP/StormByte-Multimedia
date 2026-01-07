#include <StormByte/multimedia/engine/codec.hxx>
#include <StormByte/multimedia/engine/decoder.hxx>
#include <StormByte/multimedia/engine/encoder.hxx>
#include <StormByte/multimedia/registry/codec.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia::Engine;

Codec::Codec(int codec_id, const std::string& name, const std::string& description) noexcept:
m_codec_id(codec_id), m_name(name), m_description(description) {}

std::string Codec::Name() const noexcept {
	return m_name;
}

std::string Codec::Description() const noexcept {
	return m_description;
}

ExpectedCodec Codec::Find(const std::string& name) noexcept {
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

ExpectedCodec Codec::Find(int id) noexcept {
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

ExpectedCodec Codec::Find(Type type, const std::optional<Features>& required) noexcept {
	for (const auto& entry : Registry::Codec) {
		/* Filter by type */
		if (entry.Type() != type)
			continue;

		/* Filter by features (if requested) */
		if (required.has_value()) {
			if (!entry.Features().Has(*required))
				continue;
		}

		/* Found a matching codec */
		auto expected_codec = Codec::Find(entry.ID());
		return *expected_codec;
	}

	return required.has_value() ? Unexpected(CodecNotFound(type, *required)) : Unexpected(CodecNotFound(type));
}

Decoders Codec::Decoders() const noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;
	Engine::Decoders decoders;

	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == m_codec_id && av_codec_is_decoder(c))
			decoders.push_back(Decoder(c->id, c->name));
	}

	return decoders;
}

Encoders Codec::Encoders() const noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;
	Engine::Encoders encoders;

	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == m_codec_id && av_codec_is_encoder(c))
			encoders.push_back(Encoder(c->id, c->name));
	}

	return encoders;
}