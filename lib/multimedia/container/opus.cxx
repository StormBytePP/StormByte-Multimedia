#include <multimedia/container/opus.hxx>

using namespace StormByte::Multimedia::Container;

Opus::Opus():Container(Type::Opus, "opus") {}

bool Opus::IsStreamCompatible(const Stream::Stream&) {
	return true;
}