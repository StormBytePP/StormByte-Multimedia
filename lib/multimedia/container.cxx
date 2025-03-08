#include <multimedia/container.hxx>

using namespace StormByte::Multimedia;

// Audio Containers
template<>
constexpr bool Container<Media::Container::AC3>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::AC3;
}
template<>
constexpr bool Container<Media::Container::AC3>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::FLAC>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::FLAC;
}
template<>
constexpr bool Container<Media::Container::FLAC>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::MP3>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::MP3;
}
template<>
constexpr bool Container<Media::Container::MP3>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::OGA>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::VORBIS || codec == Media::Codec::OPUS;
}

template<>
constexpr bool Container<Media::Container::WAV>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::PCM || codec == Media::Codec::WMA;
}
template<>
constexpr bool Container<Media::Container::WAV>::CanAddStreams() const noexcept {
	return streams.empty();
}

// Video Containers
template<>
constexpr bool Container<Media::Container::AVI>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::MJPEG || codec == Media::Codec::XVID;
}

template<>
constexpr bool Container<Media::Container::MKV>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::H265 || codec == Media::Codec::VP8 || 
			codec == Media::Codec::VP9 || codec == Media::Codec::AV1 || codec == Media::Codec::AAC ||
			codec == Media::Codec::FLAC || codec == Media::Codec::VORBIS || codec == Media::Codec::SUBRIP ||
			codec == Media::Codec::WEBVTT || codec == Media::Codec::OPUS;
}

template<>
constexpr bool Container<Media::Container::MP4>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::H265 || codec == Media::Codec::AV1 ||
			codec == Media::Codec::AAC || codec == Media::Codec::FLAC || codec == Media::Codec::MP3;
}

template<>
constexpr bool Container<Media::Container::M2TS>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::H265 || codec == Media::Codec::AC3 ||
			codec == Media::Codec::AAC || codec == Media::Codec::PCM;
}

template<>
constexpr bool Container<Media::Container::MPEG>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::MP3 || codec == Media::Codec::AAC;
}

template<>
constexpr bool Container<Media::Container::MPG>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::MP3 || codec == Media::Codec::AAC;
}

template<>
constexpr bool Container<Media::Container::OGV>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::THEORA || codec == Media::Codec::VORBIS;
}

template<>
constexpr bool Container<Media::Container::TS>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::H264 || codec == Media::Codec::H265 || codec == Media::Codec::AAC;
}

template<>
constexpr bool Container<Media::Container::WEBM>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	auto codec = stream->GetCodec();
	return codec == Media::Codec::VP8 || codec == Media::Codec::VP9 || codec == Media::Codec::OPUS;
}

// Image Containers
template<>
constexpr bool Container<Media::Container::BMP>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::BMP;
}
template<>
constexpr bool Container<Media::Container::BMP>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::GIF>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::GIF;
}
template<>
constexpr bool Container<Media::Container::GIF>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::HEIC>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::TIFF;
}
template<>
constexpr bool Container<Media::Container::HEIC>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::JPG>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::JPEG;
}
template<>
constexpr bool Container<Media::Container::JPG>::CanAddStreams() const noexcept {
	return streams.empty();
}

template<>
constexpr bool Container<Media::Container::PNG>::CanAddStream(const Stream::PointerType& stream) const noexcept {
	return stream->GetCodec() == Media::Codec::PNG;
}
template<>
constexpr bool Container<Media::Container::PNG>::CanAddStreams() const noexcept {
	return streams.empty();
}
	