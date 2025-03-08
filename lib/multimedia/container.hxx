#pragma once

#include <multimedia/media/container.hxx>
#include <multimedia/exception.hxx>
#include <multimedia/stream.hxx>

#include <vector>
#include <span>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Container
	 * @brief The container class.
	 * @tparam C The container type.
	 */
	template<Media::Container C>
	class STORMBYTE_MULTIMEDIA_PUBLIC Container {
		public:
			/**
			 * @brief Copy constructor.
			 * @param container The Container to copy.
			 */
			Container(const Container& container) noexcept 				= default;

			/**
			 * @brief Move constructor.
			 * @param container The Container to move.
			 */
			Container(Container&& container) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param container The Container to copy.
			 * @return The copied Container.
			 */
			Container& operator=(const Container& container) noexcept 	= default;

			/**
			 * @brief Move assignment operator.
			 * @param container The Container to move.
			 * @return The moved Container.
			 */
			Container& operator=(Container&& container) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Container() noexcept 								= default;

			/**
			 * @brief Gets the container type.
			 * @return The container type.
			 */
			constexpr Media::Container 									GetName() const noexcept {
				return C;
			}

			/**
			 * @brief Gets the streams in the container.
			 * @return The streams in the container.
			 */
			std::span<Stream::PointerType> 								GetStreams() noexcept {
				return std::span<Stream::PointerType>(streams);
			}

			/**
			 * @brief Check if the container can add streams (for example, some containers can only have one stream).
			 * @return True if the container can add streams, false otherwise.
			 */
			constexpr bool 												CanAddStreams() const noexcept {
				return true;
			}

			/**
			 * @brief Check if the container can add a stream (for example, audio containers can not have video streams).
			 * @param stream The stream to add.
			 * @return True if the container can add the stream, false otherwise.
			 */
			constexpr bool												CanAddStream(const Stream::PointerType& stream) const noexcept;

			/**
			 * @brief Add a stream to the container.
			 * @param stream The stream to add.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			void														AddStream(const Stream::PointerType& stream) noexcept {
				if (StreamCheck(stream))
					streams.push_back(stream->Clone());
			}

			/**
			 * @brief Add a stream to the container.
			 * @param stream The stream to add.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			void														AddStream(Stream::PointerType&& stream) noexcept {
				if (StreamCheck(stream))
					streams.push_back(stream->Move());
			}

		protected:
			std::vector<Stream::PointerType> streams;					///< The streams in the container.

			/**
			 * @brief Default constructor.
			 */
			Container() noexcept;

		private:
			/**
			 * @brief Check if the container can add a stream (for example, audio containers can not have video streams).
			 * @param stream The stream to add.
			 * @return True if the container can add the stream, false otherwise.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			constexpr bool												StreamCheck(const Stream::PointerType& stream) const noexcept {
				if (!CanAddStream(stream)) {
					throw ContainerIsFull(C);
				}
				else if (!CanAddStream(stream)) {
					throw CodecNotSupported(C, stream->GetCodec());
				}
				else
					return true;
			}
	};
}