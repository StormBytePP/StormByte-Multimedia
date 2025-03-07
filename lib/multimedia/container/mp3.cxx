#include <multimedia/container/mp3.hxx>

using namespace StormByte::Multimedia::Container;

MP3::MP3():Container(Type::MP3, "mp3") {}

bool MP3::IsStreamCompatible(const Stream::Stream&) {
	return true;
}