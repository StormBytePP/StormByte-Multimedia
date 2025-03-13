#pragma once

#include <StormByte/multimedia/media/container.hxx>
#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/stream.hxx>

#include <vector>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Container
	 * @brief The container class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Container final {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The container name.
			 */
			constexpr Container(const Media::Container::Name& name):m_name(name) {}

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
			~Container() noexcept 										= default;

			/**
			 * @brief Gets the container name.
			 * @return The container name.
			 */
			constexpr const Media::Container::Name&						Name() const noexcept {
				return m_name;
			}

			/**
			 * @brief Gets the streams in the container.
			 * @return The streams in the container.
			 */
			constexpr Stream::Span 										Streams() noexcept {
				return Stream::Span(m_streams);
			}

			/**
			 * @brief Gets the streams in the container.
			 * @return The streams in the container.
			 */
			constexpr Stream::ConstSpan 								Streams() const noexcept {
				return Stream::ConstSpan(m_streams);
			}

			/**
			 * @brief Gets the size of this container.
			 * @return Container size
			 */
			constexpr size_t											Size() const noexcept {
				return m_streams.size();
			}

			/**
			 * @brief Check if the container is full.
			 * @return True if the container is full, false otherwise.
			 */
			bool 														IsFull() const noexcept;

			/**
			 * @brief Checks if the codec is supported
			 * @param codec Codec to check
			 * @return True if the codec is supported, false otherwise
			 */
			bool 														Supports(const Media::Codec::Name& codec) const noexcept;

			/**
			 * @brief Check if the container can add a stream (for example, audio containers can not have video streams).
			 * @param stream The stream to add.
			 * @return True if the container can add the stream, false otherwise.
			 */
			bool														CanAddStream(const Stream::Base& stream) const noexcept;

			/**
			 * @brief Add a stream to the container.
			 * @param stream The stream to add.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			void														AddStream(const Stream::Base& stream);

			/**
			 * @brief Add a stream to the container.
			 * @param stream The stream to add.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			void														AddStream(Stream::Base&& stream);

			/**
			 * @brief Add a stream to the container.
			 * @param stream The stream to add.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			void														AddStream(std::shared_ptr<Stream::Base> stream);

		private:
			Media::Container::Name m_name;								///< The container name.
			std::vector<Stream::PointerType> m_streams;					///< The streams in the container.

			/**
			 * @brief Check if the container can add a stream (for example, audio containers can not have video streams).
			 * @param stream The stream to add.
			 * @throws ContainerIsFull If the container is full.
			 * @throws CodecNotSupported If the codec is not supported.
			 */
			void														StreamAdditionCheck(const Stream::Base& stream) const;
	};
}