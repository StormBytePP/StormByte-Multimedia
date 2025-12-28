#pragma once

#include <memory>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/dict.h>
	#include <libavutil/frame.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavfilter/avfilter.h>
	#include <libavutil/buffer.h>
	#include <libavformat/avio.h>
}

struct AVFrameDeleter { void operator()(AVFrame* p) const { av_frame_free(&p); } };
using AVFramePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;

struct AVPacketDeleter { void operator()(AVPacket* p) const { av_packet_free(&p); } };
using AVPacketPtr = std::unique_ptr<AVPacket, AVPacketDeleter>;

struct AVFormatContextDeleter { void operator()(AVFormatContext* p) const { avformat_close_input(&p); } };
using AVFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;

struct AVCodecContextDeleter { void operator()(AVCodecContext* p) const { avcodec_free_context(&p); } };
using AVCodecContextPtr = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;

struct SwsContextDeleter { void operator()(SwsContext* p) const { sws_freeContext(p); } };
using SwsContextPtr = std::unique_ptr<SwsContext, SwsContextDeleter>;

struct SwrContextDeleter { void operator()(SwrContext* p) const { swr_free(&p); } };
using SwrContextPtr = std::unique_ptr<SwrContext, SwrContextDeleter>;

struct AVDictionaryDeleter { void operator()(AVDictionary* p) const { av_dict_free(&p); } };
using AVDictionaryPtr = std::unique_ptr<AVDictionary, AVDictionaryDeleter>;

struct AVIOContextDeleter { void operator()(AVIOContext* p) const { avio_closep(&p); } };
using AVIOContextPtr = std::unique_ptr<AVIOContext, AVIOContextDeleter>;

struct AVFilterGraphDeleter { void operator()(AVFilterGraph* p) const { avfilter_graph_free(&p); } };
using AVFilterGraphPtr = std::unique_ptr<AVFilterGraph, AVFilterGraphDeleter>;

struct AVBufferRefDeleter { void operator()(AVBufferRef* p) const { av_buffer_unref(&p); } };
using AVBufferRefPtr = std::unique_ptr<AVBufferRef, AVBufferRefDeleter>;

/* Para punteros devueltos por av_malloc / av_free */
using AvFreePtr = std::unique_ptr<uint8_t, void(*)(void*)>;