#include <multimedia/container/matroska.hxx>

using namespace StormByte::Multimedia::Container;

Matroska::Matroska():Base(Type::Matroska, "mkv") {}

bool Matroska::IsStreamCompatible(const Stream::Stream&) {
	return true;
}