#pragma once

#include <StormByte/multimedia/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/ffmpeg/typedefs.hxx>

#include <deque>

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVPacket;
	/**
	 * @class AVBSFPipeline
	 * @brief Class representing a pipeline of bitstream filters (AVBSF).
	 *
	 * This class manages a sequence of AVBSF instances, allowing packets to be processed
	 * through multiple bitstream filters in order.
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED AVBSFPipeline {
		public:
			/** 
			 * @brief Default constructor.
			 */
			AVBSFPipeline() noexcept 									= default;

			/** 
			 * @brief Copy constructor (deleted).
			 * @param other The other AVBSFPipeline to copy from.
			 */
			AVBSFPipeline(const AVBSFPipeline&) 						= delete;

			/** 
			 * @brief Move constructor.
			 * @param other The other AVBSFPipeline to move from.
			 */
			AVBSFPipeline(AVBSFPipeline&& other) noexcept;

			/** 
			 * @brief Destructor.
			 */
			~AVBSFPipeline() noexcept 									= default;

			/** 
			 * @brief Copy assignment operator (deleted).
			 * @param other The other AVBSFPipeline to copy from.
			 * @return Reference to this AVBSFPipeline.
			 */
			AVBSFPipeline& operator=(const AVBSFPipeline&) 				= delete;

			/** 
			 * @brief Move assignment operator.
			 * @param other The other AVBSFPipeline to move from.
			 * @return Reference to this AVBSFPipeline.
			 */
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
