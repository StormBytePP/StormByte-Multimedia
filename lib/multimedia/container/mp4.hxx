#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class MP4
	 * @brief The MP4 container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC MP4 final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			MP4();

			/**
			 * @brief Copy constructor.
			 * @param mp4 The MP4 to copy.
			 */
			MP4(const MP4& mp4) 										= default;

			/**
			 * @brief Move constructor.
			 * @param mp4 The MP4 to move.
			 */
			MP4(MP4&& mp4) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param mp4 The MP4 to copy.
			 * @return The copied MP4.
			 */
			MP4& operator=(const MP4& mp4) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param mp4 The MP4 to move.
			 * @return The moved MP4.
			 */
			MP4& operator=(MP4&& mp4) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~MP4() noexcept override									= default;

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}