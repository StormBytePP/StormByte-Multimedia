#pragma once

#include <exception.hxx>
#include <multimedia/visibility.h>

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
}