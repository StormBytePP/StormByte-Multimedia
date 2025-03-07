#include <multimedia/container/webm.hxx>

using namespace StormByte::Multimedia::Container;

WebM::WebM():Container(Type::WebM, "webm") {}

bool WebM::IsStreamCompatible(const Stream::Stream&) {
	return true;
}