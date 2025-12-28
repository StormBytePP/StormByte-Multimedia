#pragma once

#include <StormByte/multimedia/metadata.hxx>
#include <StormByte/multimedia/typedefs.hxx>

#include <filesystem>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	class STORMBYTE_MULTIMEDIA_PUBLIC File final {
		public:
			/**
			 * @brief Copy constructor.
			 * @param other The other file to copy from.
			 */
			File(const File& other)							= default;

			/**
			 * @brief Move constructor.
			 * @param other The other file to move from.
			 */
			File(File&& other) noexcept						= default;

			/**
			 * @brief Default destructor.
			 */
			~File() noexcept								= default;

			/**
			 * @brief Get the path to the multimedia file.
			 * @return The path to the multimedia file.
			 */
			const std::filesystem::path& 					Path() const noexcept;

			/**
			 * @brief Get the metadata of the multimedia file.
			 * @return The metadata of the multimedia file.
			 */
			const class Metadata& 							Metadata() const noexcept;

			/**
			 * @brief Set the metadata of the multimedia file.
			 * @param metadata The metadata to set.
			 */
			void 											Metadata(class Metadata&& metadata) noexcept;

			/**
			 * @brief Gets the beginning iterator of the streams in the multimedia file.
			 * @return Iterator to the beginning of the streams.
			 */
			Streams::const_iterator 						StreamsBegin() const noexcept;

			/**
			 * @brief Gets the ending iterator of the streams in the multimedia file.
			 * @return Iterator to the end of the streams.
			 */
			Streams::const_iterator 						StreamsEnd() const noexcept;

			/**
			 * @brief Open a multimedia file.
			 * @param path The path to the multimedia file.
			 * @return Expected File object or FileError exception.
			 */
			static ExpectedFile 							Open(const std::filesystem::path& path) noexcept;
		
		private:
			std::filesystem::path m_path;					///< Path to the multimedia file
			class Metadata m_metadata;						///< Metadata of the multimedia file
			Streams m_streams;								///< Streams in the multimedia file

			/**
			 * @brief Constructor.
			 * @param path The path to the multimedia file.
			 */
			explicit File(const std::filesystem::path& path) noexcept;
	};
}