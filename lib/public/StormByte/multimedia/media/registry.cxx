#include <StormByte/multimedia/media/tables/codec/table.hxx>
#include <StormByte/multimedia/media/registry.hxx>

#include <string>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia::Media;

namespace {
	Access ProbeAccess(const Tables::Codec::CodecDef& def) noexcept {
		Access access(Operation::Read);
		for (std::size_t i = 0; i < def.FfmpegIdCount(); ++i) {
			const AVCodecDescriptor* desc = avcodec_descriptor_get_by_name(def.FfmpegId(i));
			if (!desc)
				continue;
			if (avcodec_find_encoder(desc->id) != nullptr) {
				access |= Access(Operation::Write);
				break;
			}
		}
		return access;
	}
}

Registry::Registry() noexcept {
	Initialize();
}

Registry& Registry::Instance() noexcept {
	static Registry instance;
	return instance;
}

CodecRefs Registry::CodecList(Type type) const noexcept {
	CodecRefs out;
	const auto it = m_by_type.find(type);
	if (it == m_by_type.end())
		return out;

	out.reserve(it->second.size());
	for (const std::size_t i : it->second)
		out.emplace_back(m_codecs[i]);
	return out;
}

ExpectedCodec Registry::FindCodec(std::string_view name) const noexcept {
	const auto it = m_by_name.find(name);
	if (it == m_by_name.end())
		return Unexpected<CodecNotFoundException>(std::string(name));

	return m_codecs[it->second];
}

void Registry::Add(Type type, const Tables::Codec::CodecDef& def) noexcept {
	const std::size_t i = m_codecs.size();
	m_codecs.push_back(Codec(type, def.name, def.description, ProbeAccess(def)));
	const Codec& stored = m_codecs[i];

	m_by_name.emplace(stored.Name(), i);
	for (std::size_t n = 0; n < def.FfmpegIdCount(); ++n)
		m_by_name.emplace(def.FfmpegId(n), i);
	m_by_type[type].push_back(i);
}

void Registry::Load(Type type, std::span<const Tables::Codec::CodecDef> table) noexcept {
	for (const Tables::Codec::CodecDef& def : table)
		Add(type, def);
}

void Registry::Initialize() noexcept {
	const auto video 		= Tables::Codec::Video();
	const auto audio 		= Tables::Codec::Audio();
	const auto subtitle 	= Tables::Codec::Subtitle();
	const auto attachment 	= Tables::Codec::Attachment();

	m_codecs.reserve(video.size() + audio.size() + subtitle.size() + attachment.size());
	m_by_name.reserve((video.size() + audio.size() + subtitle.size() + attachment.size()) * 2);
	m_by_type.reserve(4);

	Load(Type::Video, video);
	Load(Type::Audio, audio);
	Load(Type::Subtitle, subtitle);
	Load(Type::Attachment, attachment);
}
