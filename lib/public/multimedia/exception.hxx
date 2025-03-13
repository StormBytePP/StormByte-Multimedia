#pragma once

#include <exception.hxx>
#include <multimedia/media/codec.hxx>
#include <multimedia/media/container.hxx>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Exception
	 * @brief The base exception for multimedia.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param message The message of the exception.
			 */
			Exception(const std::string& message);

			/**
			 * @brief Default constructor.
			 * @param message The message of the exception.
			 */
			Exception(std::string&& message) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			Exception(const Exception& exception) 						= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			Exception(Exception&& exception) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			Exception& operator=(const Exception& exception) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			Exception& operator=(Exception&& exception) noexcept 		= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Exception() noexcept 								= default;
	};

	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotFound: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			CodecNotFound(const std::string& codec);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			CodecNotFound(const CodecNotFound& exception) 						= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			CodecNotFound(CodecNotFound&& exception) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			CodecNotFound& operator=(const CodecNotFound& exception) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			CodecNotFound& operator=(CodecNotFound&& exception) noexcept 		= default;

			/**
			 * @brief Default destructor.
			 */
			~CodecNotFound() noexcept override									= default;
	};

	class STORMBYTE_MULTIMEDIA_PUBLIC ContainerNotFound: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param container The message of the exception.
			 */
			ContainerNotFound(const std::string& container);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			ContainerNotFound(const ContainerNotFound& exception) 					= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			ContainerNotFound(ContainerNotFound&& exception) noexcept 				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			ContainerNotFound& operator=(const ContainerNotFound& exception) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			ContainerNotFound& operator=(ContainerNotFound&& exception) noexcept 	= default;

			/**
			 * @brief Default destructor.
			 */
			~ContainerNotFound() noexcept override									= default;
	};

	/**
	 * @class CodecNotSupported
	 * @brief The exception for when a codec is not supported for a container
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotSupported: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param container The container.
			 * @param codec The Codec.
			 */
			CodecNotSupported(const Media::Container::Name& container, const Media::Codec::Name& codec);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			CodecNotSupported(const CodecNotSupported& exception) 						= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			CodecNotSupported(CodecNotSupported&& exception) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			CodecNotSupported& operator=(const CodecNotSupported& exception) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			CodecNotSupported& operator=(CodecNotSupported&& exception) noexcept 		= default;

			/**
			 * @brief Default destructor.
			 */
			~CodecNotSupported() noexcept override										= default;
	};

	class STORMBYTE_MULTIMEDIA_PUBLIC ContainerIsFull: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param container The container.
			 */
			ContainerIsFull(const Media::Container::Name& container);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			ContainerIsFull(const ContainerIsFull& exception) 							= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			ContainerIsFull(ContainerIsFull&& exception) noexcept 						= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			ContainerIsFull& operator=(const ContainerIsFull& exception) 				= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			ContainerIsFull& operator=(ContainerIsFull&& exception) noexcept 			= default;

			/**
			 * @brief Default destructor.
			 */
			~ContainerIsFull() noexcept override										= default;
	};
}