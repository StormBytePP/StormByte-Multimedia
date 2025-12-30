#include <StormByte/multimedia/features.hxx>

using namespace StormByte::Multimedia;

Features::operator std::string() const noexcept {
	std::string result;
	bool first = true;

	for (StormByte::Type::UnderlyingType<Feature> bit = 1; bit != 0; bit <<= 1) {
		std::string result;
		bool first = true;
		const std::string separator = " | ";

		for (StormByte::Type::UnderlyingType<Feature> bit = 1; bit != 0; bit <<= 1) {
			Feature feature = static_cast<Feature>(bit);
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
