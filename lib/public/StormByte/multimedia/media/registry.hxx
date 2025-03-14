#pragma once

#include <StormByte/alias.hxx>
#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/codec.hxx>
#include <StormByte/multimedia/media/container.hxx>

/**
 * @class Registry
 * @brief Centralized registry to manage codec-related metadata.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @class Registry
	 * @brief Centralized registry to manage codec-related metadata.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Registry {
		/**
		 * @brief Retrieves detailed information about a codec.
		 * @param codec The codec enum value.
		 * @return A reference to the Codec::Info struct for the requested codec.
		 */
		static const Codec::Info& CodecInfo(const Codec::Name& codec);

		/**
		 * @brief Retrieves detailed information about a container.
		 * @param container The container enum value.
		 * @return A reference to the Container::Info struct for the requested container.
		 */
		static const Container::Info& ContainerInfo(const Container::Name& container);

		/**
		 * @brief Retrieves detailed information about a codec by name.
		 * @param codecName The name of the codec.
		 * @return A reference to the CodecInfo struct for the requested codec.
		 */
		static StormByte::Expected<Codec::Name, CodecNotFound> CodecInfo(const std::string& codecName);

		/**
		 * @brief Retrieves detailed information about a codec by name.
		 * @param codecName The name of the codec.
		 * @return A reference to the CodecInfo struct for the requested codec.
		 */
		static StormByte::Expected<Container::Name, ContainerNotFound> ContainerInfo(const std::string& codecName);

		private:
			static const std::unordered_map<Codec::Name, Codec::Info> c_codec_registry; 			///< The codec registry.
			static const std::unordered_map<std::string, Codec::Name> c_codec_name_map;				///< The codec name map.
			static const std::unordered_map<Container::Name, Container::Info> c_container_registry;	///< Registry of container metadata.
			static const std::unordered_map<std::string, Container::Name> c_container_name_map;		///< Reverse lookup map.
	};
}