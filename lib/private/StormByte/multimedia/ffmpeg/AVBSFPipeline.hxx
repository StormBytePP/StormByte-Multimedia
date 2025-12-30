#pragma once

#include <StormByte/multimedia/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/ffmpeg/typedefs.hxx>

#include <deque>
#include <optional>

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVPacket;
	class STORMBYTE_MULTIMEDIA_PRIVATE AVBSFPipeline {
		public:
			AVBSFPipeline() noexcept 									= default;
			AVBSFPipeline(const AVBSFPipeline&) 						= delete;
			AVBSFPipeline(AVBSFPipeline&& other) noexcept;
			~AVBSFPipeline() noexcept 									= default;

			AVBSFPipeline& operator=(const AVBSFPipeline&) 				= delete;
			AVBSFPipeline& operator=(AVBSFPipeline&& other) noexcept;

			/** 
			 * @brief Adds a bitstream filter to the pipeline.
			 * @param bsf The bitstream filter to add.
			 */
			void 														Add(AVBSF&& bsf) noexcept;

			/** 
			 * @brief Processes a packet through the pipeline of bitstream filters.
			 * @param pkt The packet to process.
			 * @return OperationResult The result of the processing operation.
			 */
			OperationResult 											Process(AVPacket& pkt) noexcept;

			/** 
			 * @brief Flushes all bitstream filters in the pipeline.
			 */
			void 														Flush() noexcept;

			/** 
			 * @brief Sets end-of-file on all bitstream filters in the pipeline.
			 */
			void 														SetEof() noexcept;

			/** 
			 * @brief Clears all bitstream filters from the pipeline.
			 */
			void 														Clear() noexcept;

			/** 
			 * @brief Checks if the pipeline is empty.
			 * @return true if the pipeline is empty, false otherwise.
			 */
			bool 														Empty() const noexcept;

		private:
			std::deque<AVBSF> m_filters;								///< The deque of bitstream filters in the pipeline.
	};
}
