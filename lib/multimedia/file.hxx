#pragma once

#include <multimedia/container/container.hxx>
#include <multimedia/property/size.hxx>

#include <filesystem>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class File
	 * @brief The multimedia file class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC File {
		public:
			/**
			 * @brief Default constructor.
			 * @param path The path to the file.
			 */
			File(const std::filesystem::path& path);

			/**
			 * @brief Copy constructor.
			 * @param file The file to copy.
			 */
			File(const File& file) 								= default;

			/**
			 * @brief Move constructor.
			 * @param file The file to move.
			 */
			File(File&& file) noexcept 							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param file The file to copy.
			 * @return The copied file.
			 */
			File& operator=(const File& file) 					= default;

			/**
			 * @brief Move assignment operator.
			 * @param file The file to move.
			 * @return The moved file.
			 */
			File& operator=(File&& file) noexcept 				= default;

			/**
			 * @brief Default destructor.
			 */
			~File() noexcept									= default;

			/**
			 * @brief Gets the path to the file.
			 * @return The path to the file.
			 */
			const std::filesystem::path& 						GetPath() const noexcept;

			/**
			 * @brief Gets the size of the file.
			 * @return The size of the file.
			 */
			const Property::Size& 								GetSize() const noexcept;

			/**
			 * @brief Gets the container of the file.
			 * @return The container of the file.
			 */
			std::shared_ptr<Container::Container>				GetContainer() const noexcept;

		protected:
			std::filesystem::path m_path;						///< The path to the file.
			Property::Size m_size;								///< The size of the file.
			std::shared_ptr<Container::Container> m_container;	///< The container of the file.
	};
}