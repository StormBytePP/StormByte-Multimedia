#include <multimedia/container/flac.hxx>

using namespace StormByte::Multimedia::Container;

FLAC::FLAC():Container(Type::FLAC, "flac") {}

bool FLAC::IsStreamCompatible(const Stream::Stream&) {
	return true;
}