#include <StormByte/multimedia/features.hxx>

using namespace StormByte::Multimedia;

Features::operator std::string() const noexcept {
	std::string result;
	bool first = true;
	const std::string separator = " | ";

	for (uint8_t cat = 1; cat <= 5; ++cat) {
		for (uint8_t idx = 1; idx <= 8; ++idx) {
			Feature feature = static_cast<Feature>((cat << 4) | idx);
			if (Has(feature)) {
				if (!first)
					result += separator + ToString(feature);
				else {
					first = false;
					result += ToString(feature);
				}
			}
		}
	}

	return result;
}

// ------------------------------------------------------------
// Explicit instantiations for Feature bitwise operators
// ------------------------------------------------------------
template STORMBYTE_MULTIMEDIA_PUBLIC Feature operator|(Feature, Feature);
template STORMBYTE_MULTIMEDIA_PUBLIC Feature operator&(Feature, Feature);
template STORMBYTE_MULTIMEDIA_PUBLIC Feature operator^(Feature, Feature);
template STORMBYTE_MULTIMEDIA_PUBLIC Feature operator~(Feature);

// ------------------------------------------------------------
// Explicit instantiation for Features Bitmask
// ------------------------------------------------------------
template class STORMBYTE_MULTIMEDIA_PUBLIC StormByte::Bitmask<StormByte::Multimedia::Features, StormByte::Multimedia::Feature>;
