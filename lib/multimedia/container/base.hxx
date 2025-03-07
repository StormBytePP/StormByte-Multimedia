#pragma once

#include <util/templates/iterator.hxx>
#include <multimedia/container/type.hxx>
#include <multimedia/stream/stream.hxx>
#include <multimedia/property/size.hxx>

#include <filesystem>
#include <vector>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class Base
	 * @brief The base class for all containers.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Base {
		public:
			using Streams 		= std::vector<std::shared_ptr<Stream::Stream>>;		///< Representation for a vector of streams.
			using Iterator 		= Util::Templates::Iterator<Streams>;				///< Representation for an iterator of streams.
			using ConstIterator	= Util::Templates::ConstIterator<Streams>;			///< Representation for a const iterator of streams.

			/**
			 * @brief Default constructor.
			 * @param type The type of the container.
			 */
			Base(const Type& type, const std::string& extension);

			/**
			 * @brief Copy constructor.
			 * @param container The Base to copy.
			 */
			Base(const Base& container) 									= default;

			/**
			 * @brief Move constructor.
			 * @param container The Base to move.
			 */
			Base(Base&& container) noexcept 								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param container The Base to copy.
			 * @return The copied Base.
			 */
			Base& operator=(const Base& container) 						= default;

			/**
			 * @brief Move assignment operator.
			 * @param container The Base to move.
			 * @return The moved Base.
			 */
			Base& operator=(Base&& container) noexcept 					= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Base() noexcept											= default;

			/**
			 * @brief Gets the type of the container.
			 * @return The type of the container.
			 */
			const Type& 															GetType() const noexcept;

			/**
			 * @brief Gets the size of the container.
			 * @return The size of the container.
			 */
			const Property::Size& 													GetSize() const noexcept;

			/**
			 * @brief Adds a stream to the container.
			 * @param stream The stream to add.
			 * @throw StreamNotCompatible If the stream is not compatible with the container.
			 * @throw CantAddStreams If the container cannot add streams.
			 */
			void 																	AddStream(const Stream::Stream& stream);

			/**
			 * @brief Adds a stream to the container.
			 * @param stream The stream to add.
			 * @throw StreamNotCompatible If the stream is not compatible with the container.
			 * @throw CantAddStreams If the container cannot add streams.
			 */
			void 																	AddStream(Stream::Stream&& stream);

			/**
			 * @brief Gets the count of streams in the container.
			 * @return The count of streams in the container.
			 */
			size_t 																	GetStreamCount() const noexcept;

			/**
			 * @brief Gets the extension of the container.
			 * @return The extension of the container.
			 */
			const std::string&														GetExtension() const noexcept;

			/**
			 * @brief Gets the begin iterator.
			 * @return The begin iterator.
			 */
			Iterator 																Begin() noexcept;

			/**
			 * @brief Gets the begin iterator.
			 * @return The begin iterator.
			 */
			ConstIterator 															CBegin() const noexcept;

			/**
			 * @brief Gets the end iterator.
			 * @return The end iterator.
			 */
			Iterator 																End() noexcept;

			/**
			 * @brief Gets the end iterator.
			 * @return The end iterator.
			 */
			ConstIterator 															CEnd() const noexcept;

			/**
			 * @brief Gets the begin iterator.
			 * @return The begin iterator.
			 */
			ConstIterator 															Begin() const noexcept;

			/**
			 * @brief Gets the end iterator.
			 * @return The end iterator.
			 */
			ConstIterator 															End() const noexcept;

			/**
			 * @brief Creates a container.
			 * @param type The type of the container.
			 * @return The created container.
			 */
			static std::shared_ptr<Base> 										Create(const Type& type);

			/**
			 * @brief Creates a container.
			 * @param extension The extension of the container.
			 * @return The created container.
			 */
			static std::shared_ptr<Base> 										Create(const std::string& extension);

		protected:
			Type m_type;															///< The type of the container.
			std::string m_extension;												///< The extension of the container.
			Streams m_streams;														///< The streams in the container (they are ordered!).

			/**
			 * @brief Checks if the stream is compatible with the container.
			 * @param stream The stream to check.
			 * @return True if the stream is compatible with the container, false otherwise.
			 */
			virtual bool 															IsStreamCompatible(const Stream::Stream& stream) = 0;

			/**
			 * @brief Checks if the container can add streams, for example a MP3 which has already a stream cannot add more.
			 * @return True if the container can add streams, false otherwise. (default is true)
			 */
			virtual bool 															CanAddStreams() const noexcept;
	};
}