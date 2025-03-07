#include <multimedia/container/webm.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	WebM::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	WebM::CompatCodecs {};

WebM::WebM():Base(Type::WebM, "webm") {}
