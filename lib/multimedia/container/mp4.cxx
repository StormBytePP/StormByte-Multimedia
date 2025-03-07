#include <multimedia/container/mp4.hxx>

using namespace StormByte::Multimedia::Container;

MP4::MP4():Base(Type::MP4, "mp4") {}

bool MP4::IsStreamCompatible(const Stream::Stream&) {
	return true;
}