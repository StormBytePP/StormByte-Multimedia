#include <StormByte/multimedia/engine/backend/ffmpeg/demuxer.hxx>
#include <StormByte/multimedia/engine/demuxer.hxx>

using namespace StormByte::Multimedia::Engine;

Demuxer::Demuxer(const std::filesystem::path& path, class Metadata&& metadata, class Streams&& streams) noexcept
:m_path(path), m_metadata(std::move(metadata)), m_streams(std::move(streams)) {}

ExpectedDemuxer Demuxer::Open(const std::filesystem::path& path, enum Implementation implementation) noexcept {
	if (!ImplementedBackends().contains(implementation)) {
		return Unexpected<BackendNotImplemented>(ToString(implementation));
	}

	// Select backend
	switch(implementation) {
		case Implementation::FFmpeg: {
			// Create FFmpeg demuxer
			Engine::Backend::FFmpeg::Demuxer ffmpeg_demuxer;

			// Open demuxer and get components
			auto expected_tuple = ffmpeg_demuxer.Open(path);
			if (!expected_tuple)
				return Unexpected(expected_tuple.error());

			// Unpack tuple
			auto [metadata, streams] = expected_tuple.value();

			// Create Demuxer object
			return Demuxer(path, std::move(metadata), std::move(streams));
		}

		default:
			return Unexpected<BackendNotImplemented>(ToString(implementation));
	}
}