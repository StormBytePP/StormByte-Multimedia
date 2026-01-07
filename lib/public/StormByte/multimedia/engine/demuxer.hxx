#pragma once

#include <StormByte/multimedia/engine/implementation.hxx>
#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>

/**
 * @namespace Engine
 * @brief The namespace for all multimedia engine classes.
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Demuxer
	 * @brief The class representing a multimedia demuxer.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demuxer final {
		public:
			/**
			 * @brief Copy constructor.
			 * @param other The other file to copy from.
			 */

			Demuxer(const Demuxer& other)							= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other file to move from.
			 */
			Demuxer(Demuxer&& other) noexcept						= default;

			/**
			 * @brief Default destructor.
			 */
			~Demuxer() noexcept										= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other file to copy from.
			 * @return Reference to this file.
			 */

			Demuxer& operator=(const Demuxer& other)				= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other file to move from.
			 * @return Reference to this file.
			 */
			Demuxer& operator=(Demuxer&& other) noexcept			= default;

			/**
			 * @brief Get the path to the multimedia file.
			 * @return The path to the multimedia file.
			 */
			const std::filesystem::path& 							File() const noexcept;

			/**
			 * @brief Get the metadata of the multimedia file.
			 * @return The metadata of the multimedia file.
			 */
			inline const class Metadata& 							Metadata() const noexcept {
				return m_metadata;
			}

			/**
			 * @brief Get the streams of the multimedia file.
			 * @return The streams of the multimedia file.
			 */
			inline const class Streams& 							Streams() const noexcept {
				return m_streams;
			}

			/**
			 * @brief Open a multimedia file and create a Demuxer object.
			 * @param path The path to the multimedia file.
			 * @param backend The backend to use for demuxing.
			 * @return ExpectedDemuxer The created Demuxer or an error.
			 */
			static ExpectedDemuxer 									Open(const std::filesystem::path& path, enum Implementation implementation) noexcept;
		
		private:
			std::filesystem::path m_path;							///< Path to the multimedia file
			class Metadata m_metadata;								///< Metadata of the multimedia file
			class Streams m_streams;								///< Streams in the multimedia file

			/**
			 * @brief Private constructor used by the Open function.
			 * @param backend The backend used by the demuxer.
			 * @param path The path to the multimedia file.
			 * @param metadata The metadata of the multimedia file.
			 * @param streams The streams in the multimedia file.
			 */
			Demuxer(const std::filesystem::path& path, class Metadata&& metadata, class Streams&& streams) noexcept;
	};
}