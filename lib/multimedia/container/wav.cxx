#include <multimedia/container/wav.hxx>

using namespace StormByte::Multimedia::Container;

WAV::WAV():Container(Type::WAV, "wav") {}

bool WAV::IsStreamCompatible(const Stream::Stream&) {
	return true;
}