#include <multimedia/container/matroska.hxx>

using namespace StormByte::Multimedia::Container;

Matroska::Matroska():Container(Type::Matroska, "mkv") {}

bool Matroska::IsStreamCompatible(const Stream::Stream&) {
	return true;
}