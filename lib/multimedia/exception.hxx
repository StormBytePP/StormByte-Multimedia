#pragma once

#include <exception.hxx>
#include <multimedia/property/type.hxx>
#include <multimedia/container/type.hxx>

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

	class STORMBYTE_MULTIMEDIA_PUBLIC StreamNotCompatible: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param container The container name.
			 */
			StreamNotCompatible(const Container::Type& container);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			StreamNotCompatible(const StreamNotCompatible& exception) 						= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			StreamNotCompatible(StreamNotCompatible&& exception) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			StreamNotCompatible& operator=(const StreamNotCompatible& exception) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			StreamNotCompatible& operator=(StreamNotCompatible&& exception) noexcept 		= default;

			/**
			 * @brief Default destructor.
			 */
			~StreamNotCompatible() noexcept 												= default;
	};

	/**
	 * @class CodecNotCompatible
	 * @brief The exception for when a codec is not compatible.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotCompatible: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The codec name.
			 * @param container The container name.
			 */
			CodecNotCompatible(const std::string& codec, const std::string& container);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			CodecNotCompatible(const CodecNotCompatible& exception) 					= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			CodecNotCompatible(CodecNotCompatible&& exception) noexcept 				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			CodecNotCompatible& operator=(const CodecNotCompatible& exception) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			CodecNotCompatible& operator=(CodecNotCompatible&& exception) noexcept 		= default;

			/**
			 * @brief Default destructor.
			 */
			~CodecNotCompatible() noexcept override										= default;
	};

	/**
	 * @class CantAddStreams
	 * @brief The exception for when a container cannot add streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CantAddStreams: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param container The container name.
			 */
			CantAddStreams(const Container::Type& container);

			/**
			 * @brief Copy constructor.
			 * @param exception The exception to copy.
			 */
			CantAddStreams(const CantAddStreams& exception) 								= default;

			/**
			 * @brief Move constructor.
			 * @param exception The exception to move.
			 */
			CantAddStreams(CantAddStreams&& exception) noexcept 							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param exception The exception to copy.
			 * @return The copied exception.
			 */
			CantAddStreams& operator=(const CantAddStreams& exception) 						= default;

			/**
			 * @brief Move assignment operator.
			 * @param exception The exception to move.
			 * @return The moved exception.
			 */
			CantAddStreams& operator=(CantAddStreams&& exception) noexcept 					= default;

			/**
			 * @brief Default destructor.
			 */
			~CantAddStreams() noexcept 														= default;
	};
}