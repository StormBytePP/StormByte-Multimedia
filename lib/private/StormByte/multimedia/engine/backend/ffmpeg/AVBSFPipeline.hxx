#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>

#include <deque>

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVPacket;

	/**
	 * @class AVBSFPipeline
	 * @brief Ordered chain of bitstream filters applied to packets.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVBSFPipeline {
		public:
			/**
			 * Default constructor.
			 */
			AVBSFPipeline() noexcept = default;

			/**
			 * Copy constructor (deleted).
			 */
			AVBSFPipeline(const AVBSFPipeline&) = delete;

			/**
			 * Move constructor.
			 */
			AVBSFPipeline(AVBSFPipeline&& other) noexcept;

			/**
			 * Destructor.
			 */
			~AVBSFPipeline() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			AVBSFPipeline& operator=(const AVBSFPipeline&) = delete;

			/**
			 * Move assignment.
			 */
			AVBSFPipeline& operator=(AVBSFPipeline&& other) noexcept;

			/**
			 * Appends a filter to the chain.
			 * @param bsf Filter to take ownership of.
			 */
			void Add(AVBSF&& bsf) noexcept;

			/**
			 * Runs @p pkt through all filters in order.
			 * @param pkt Packet in/out.
			 * @return Operation result.
			 */
			OperationResult Process(AVPacket& pkt) noexcept;

			/**
			 * Flushes every filter.
			 */
			void Flush() noexcept;

			/**
			 * Signals EOF on every filter.
			 */
			void SetEof() noexcept;

			/**
			 * Removes all filters.
			 */
			void Clear() noexcept;

			/**
			 * @return true if no filters are registered.
			 */
			bool Empty() const noexcept;

		private:
			std::deque<AVBSF> m_filters;	///< Filter chain
	};
}
