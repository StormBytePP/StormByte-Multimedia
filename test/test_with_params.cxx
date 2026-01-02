#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/context/audio.hxx>
#include <StormByte/multimedia/context/video.hxx>

#include <iostream>
#include <memory>

using namespace StormByte::Multimedia;

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " <multimedia_file_path>" << std::endl;
		// Return 0 so ctest still works without parameters as a noop test
		return 0;
	}
	auto expected_file = File::Open(argv[1]);
	if (!expected_file) {
		std::cerr << "Error opening file: " << expected_file.error()->what() << std::endl;
		return 1;
	}
	for (auto stream_it = expected_file->StreamsBegin(); stream_it != expected_file->StreamsEnd(); ++stream_it) {
		const Stream& stream = *stream_it;
		std::cout << "Stream Type: ";
		switch (stream.Type()) {
			case StormByte::Multimedia::Type::Audio: {
				std::cout << "Audio" << std::endl;
				const Codec& codec = stream.Codec();
				std::cout << "  Codec: " << codec.Name() << " - " << codec.Description() << std::endl;
				auto ctx = stream.Context();
				if (auto audio = std::dynamic_pointer_cast<const Context::Audio>(ctx)) {
					std::cout << "  SampleRate: " << audio->SampleRate() << std::endl;
					std::cout << "  Channels: " << audio->Channels() << std::endl;
					std::cout << "  Bitrate: " << audio->Bitrate() << std::endl;
					if (audio->Profile())
						std::cout << "  Profile: " << *audio->Profile() << std::endl;
				}
				break;
			}
			case StormByte::Multimedia::Type::Video: {
				std::cout << "Video" << std::endl;
				const Codec& codec = stream.Codec();
				std::cout << "  Codec: " << codec.Name() << " - " << codec.Description() << std::endl;
				auto ctx = stream.Context();
				if (auto video = std::dynamic_pointer_cast<const Context::Video>(ctx)) {
					const auto& res = video->Resolution();
					const auto& color = video->Color();
					std::cout << "  Resolution: " << res.Name() << " (" << res.Width() << "x" << res.Height() << ")" << std::endl;
					std::cout << "  PixelFormat: " << color.PixelFormat() << std::endl;
					std::cout << "  Range: " << color.Range() << std::endl;
					std::cout << "  Space: " << color.Space() << std::endl;
					std::cout << "  Primaries: " << color.Primaries() << std::endl;
					std::cout << "  Transfer: " << color.Transfer() << std::endl;
					if (video->HDR10()) {
						const auto& h = *video->HDR10();
						std::cout << "  HDR10Plus: " << (h.IsHDR10Plus() ? "yes" : "no") << std::endl;
						const auto& red = h.Red();
						const auto& green = h.Green();
						const auto& blue = h.Blue();
						const auto& white = h.White();
						std::cout << "  Red Point: (" << red.X() << ", " << red.Y() << ")" << std::endl;
						std::cout << "  Green Point: (" << green.X() << ", " << green.Y() << ")" << std::endl;
						std::cout << "  Blue Point: (" << blue.X() << ", " << blue.Y() << ")" << std::endl;
						std::cout << "  White Point: (" << white.X() << ", " << white.Y() << ")" << std::endl;
						const auto& lum = h.Luminance();
						std::cout << "  Luminance: (" << lum.X() << ", " << lum.Y() << ")" << std::endl;
						if (h.LightLevel()) {
							const auto& ll = *h.LightLevel();
							std::cout << "  LightLevel: (" << ll.X() << ", " << ll.Y() << ")" << std::endl;
						}
					}
				}
				break;
			}
			case StormByte::Multimedia::Type::Subtitle:
				std::cout << "Subtitle" << std::endl;
				std::cout << "  Codec: " << stream.Codec().Name() << " - " << stream.Codec().Description() << std::endl;
				break;
			case StormByte::Multimedia::Type::Attachment:
				std::cout << "Attachment" << std::endl;
				std::cout << "  Codec: " << stream.Codec().Name() << " - " << stream.Codec().Description() << std::endl;
				for (const auto& [key, value] : stream.Metadata()) {
					std::cout << "  Metadata: " << key << " = " << value << std::endl;
				}
				break;
			default:
				std::cout << "Unknown" << std::endl;
				break;
		}
	}
	return 0;
}