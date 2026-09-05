#include <StormByte/multimedia/engine/OCR/tessdata.hxx>
#include <tessdata.h>

namespace StormByte::Multimedia::Engine::OCR {
	ExpectedTessData TessData(std::string_view language) {
		const auto data = tessdata::blob(language);
		if (data.empty())
			return Unexpected<TessDataNotFoundException>(std::string(language));
		return data;
	}
}
